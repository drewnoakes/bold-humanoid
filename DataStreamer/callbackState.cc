#include "datastreamer.ih"

int DataStreamer::callback_state(
  libwebsocket_context* context,
  libwebsocket* wsi,
  libwebsocket_callback_reasons reason,
  void* session,
  void* /*in*/,
  size_t /*len*/)
{
  JsonSession* jsonSession = reinterpret_cast<JsonSession*>(session);

  switch (reason)
  {
  case LWS_CALLBACK_ESTABLISHED:
  {
    ASSERT(ThreadUtil::isDataStreamerThread());

    const libwebsocket_protocols* protocol = libwebsockets_get_protocol(wsi);

    log::verbose(protocol->name) << "Client connected";

    // New client connected; initialize session
    new (jsonSession) JsonSession(protocol->name, wsi, context);

    std::lock_guard<std::mutex> guard(d_stateSessionsMutex);
    d_stateSessions.insert(make_pair(protocol->name, jsonSession));

    if (d_stateSessions.count(protocol->name) == 1)
      hasClientChanged(protocol->name, true);

    // Some state objects change very infrequently (such as StaticHardwareState)
    // and so we send the latest object to a client when they connect.

    // Find any existing state object
    shared_ptr<StateObject const> stateObject = State::getByName(protocol->name);

    if (stateObject)
    {
      // Encode and enqueue for this client
      WebSocketBuffer buffer;
      Writer<WebSocketBuffer> writer(buffer);
      stateObject->writeJson(writer);
      jsonSession->enqueue(move(buffer));
    }

    return 0;
  }
  case LWS_CALLBACK_CLOSED:
  {
    // Client disconnected
    ASSERT(ThreadUtil::isDataStreamerThread());

    const libwebsocket_protocols* protocol = libwebsockets_get_protocol(wsi);

    log::verbose(protocol->name) << "Client disconnected";

    std::lock_guard<std::mutex> guard(d_stateSessionsMutex);

    auto range = d_stateSessions.equal_range(protocol->name);
    for (auto it = range.first; it != range.second; ++it)
    {
      if (it->second == jsonSession)
      {
        // Found the session to remove. Remove it.
        d_stateSessions.erase(it);
        if (d_stateSessions.count(protocol->name) == 0)
          hasClientChanged(protocol->name, false);
        jsonSession->~JsonSession();
        return 0;
      }
    }

    log::error("DataStreamer::callbackState") << "LWS callback closed for unknown session";
    jsonSession->~JsonSession();
    return 0;
  }
  case LWS_CALLBACK_SERVER_WRITEABLE:
  {
    ASSERT(ThreadUtil::isDataStreamerThread());

    std::lock_guard<std::mutex> guard(d_stateSessionsMutex);
    return jsonSession->write();
  }
  default:
  {
    return 0;
  }
  }
}
