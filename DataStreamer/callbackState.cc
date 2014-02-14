#include "datastreamer.ih"

int DataStreamer::callback_state(
  libwebsocket_context* context,
  libwebsocket *wsi,
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
    assert(ThreadUtil::isDataStreamerThread());

    // New client connected; initialize session
    jsonSession->initialise();
    const libwebsocket_protocols* protocol = libwebsockets_get_protocol(wsi);
    std::lock_guard<std::mutex> guard(d_stateSessionsMutex);
    d_stateSessions.insert(make_pair(protocol->name, jsonSession));

    // Some state objects change very infrequently (such as StaticHardwareState)
    // and so we send the latest object to a client when they connect.

    // Find any existing state object
    shared_ptr<StateObject const> stateObject = AgentState::getByName(protocol->name);

    if (stateObject)
    {
      // A state object exists

      // Generate JSON bytes
      StringBuffer buffer;
      Writer<StringBuffer> writer(buffer);
      stateObject->writeJson(writer);
      auto bytes = JsonSession::createBytes(buffer);

      // Enqueue it for this client
      jsonSession->queue.push(bytes);

      libwebsocket_callback_on_writable(context, wsi);
    }

    return 0;
  }
  case LWS_CALLBACK_CLOSED:
  {
    // Client disconnected
    assert(ThreadUtil::isDataStreamerThread());
    const libwebsocket_protocols* protocol = libwebsockets_get_protocol(wsi);
    std::lock_guard<std::mutex> guard(d_stateSessionsMutex);
    auto range = d_stateSessions.equal_range(protocol->name);
    for (auto it = range.first; it != range.second; ++it)
    {
      if (it->second == jsonSession)
      {
        // Found the session to remove. Remove it.
        d_stateSessions.erase(it);
        return 0;
      }
    }
    log::error("DataStreamer::callbackState") << "LWS callback closed for unknown session";
    return 0;
  }
  case LWS_CALLBACK_SERVER_WRITEABLE:
  {
    assert(ThreadUtil::isDataStreamerThread());
    std::lock_guard<std::mutex> guard(d_stateSessionsMutex);
    return jsonSession->write(wsi, context);
  }
  default:
  {
    return 0;
  }
  }
}
