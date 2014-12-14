#include "datastreamer.ih"

int DataStreamer::callback_control(
  libwebsocket_context* context,
  libwebsocket* wsi,
  libwebsocket_callback_reasons reason,
  void* session,
  void* in,
  size_t len)
{
  JsonSession* jsonSession = reinterpret_cast<JsonSession*>(session);

  switch (reason)
  {
  case LWS_CALLBACK_ESTABLISHED:
  {
    // New client connected; initialize session
    ASSERT(ThreadUtil::isDataStreamerThread());

    log::verbose("control-protocol") << "Client connected";

    new (jsonSession) JsonSession("control-protocol", wsi, context);

    lock_guard<mutex> guard(d_controlSessionsMutex);

    d_controlSessions.push_back(jsonSession);

    if (d_controlSessions.size() == 1)
      hasClientChanged("control-protocol", true);

    // Write control sync message, with current snapshot of state
    WebSocketBuffer buffer;
    Writer<WebSocketBuffer> writer(buffer);
    writeControlSyncJson(writer);
    jsonSession->enqueue(move(buffer));

    return 0;
  }
  case LWS_CALLBACK_CLOSED:
  {
    // Client disconnected
    ASSERT(ThreadUtil::isDataStreamerThread());

    log::verbose("control-protocol") << "Client disconnecting";

    lock_guard<mutex> guard(d_controlSessionsMutex);

    d_controlSessions.erase(find(d_controlSessions.begin(), d_controlSessions.end(), jsonSession));

    jsonSession->~JsonSession();

    if (d_controlSessions.size() == 0)
      hasClientChanged("control-protocol", false);

    break;
  }
  case LWS_CALLBACK_SERVER_WRITEABLE:
  {
    // Fill the outbound pipe with frames of data
    ASSERT(ThreadUtil::isDataStreamerThread());
    return jsonSession->write();
  }
  case LWS_CALLBACK_RECEIVE:
  {
    ASSERT(ThreadUtil::isDataStreamerThread());
    if (len != 0)
    {
      log::verbose("control-protocol") << "Receiving data";
      static string message;
      message.append((char const*)in, len);

      if (libwebsockets_remaining_packet_payload(wsi) == 0)
      {
        // We now have all the data
        processCommand(message, jsonSession);

        message.clear();
      }
    }
    break;
  }
  default:
    // Unknown reason
    break;
  }

  return 0;
}
