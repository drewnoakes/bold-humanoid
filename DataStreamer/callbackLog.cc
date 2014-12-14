#include "datastreamer.ih"

int DataStreamer::callback_log(
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
    // New client connected; initialize session
    ASSERT(ThreadUtil::isDataStreamerThread());

    log::verbose("log-callback") << "Client connected";

    new (jsonSession) JsonSession("log-callback", wsi, context);

    if (d_logAppender->addSession(jsonSession) == 1)
      hasClientChanged("log-callback", true);

    // Write control sync message, with current snapshot of state
    WebSocketBuffer buffer;
    Writer<WebSocketBuffer> writer(buffer);
    d_logAppender->writeLogSyncJson(writer);
    jsonSession->enqueue(move(buffer));

    return 0;
  }
  case LWS_CALLBACK_CLOSED:
  {
    // Client disconnected
    ASSERT(ThreadUtil::isDataStreamerThread());

    log::verbose("log-callback") << "Client disconnecting";

    if (d_logAppender->removeSession(jsonSession) == 0)
      hasClientChanged("log-callback", false);

    break;
  }
  case LWS_CALLBACK_SERVER_WRITEABLE:
  {
    // Fill the outbound pipe with frames of data
    ASSERT(ThreadUtil::isDataStreamerThread());
    return jsonSession->write();
  }
  default:
    // Unknown reason
    break;
  }

  return 0;
}
