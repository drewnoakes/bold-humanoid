#include "datastreamer.ih"

int DataStreamer::callback_control(
  libwebsocket_context* context,
  libwebsocket *wsi,
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
    assert(ThreadUtil::isDataStreamerThread());
    jsonSession->initialise();
    d_controlSessions.push_back(jsonSession);
    if (d_controlSessions.size() == 1)
      hasClientChanged("control-protocol", true);

    // Write control sync message, with current snapshot of state
    jsonSession->queue.push(prepareControlSyncBytes());
    libwebsocket_callback_on_writable(context, wsi);

    return 0;
  }
  case LWS_CALLBACK_CLOSED:
  {
    // Client disconnected
    assert(ThreadUtil::isDataStreamerThread());
    d_controlSessions.erase(find(d_controlSessions.begin(), d_controlSessions.end(), jsonSession));
    if (d_controlSessions.size() == 0)
      hasClientChanged("control-protocol", false);
    break;
  }
  case LWS_CALLBACK_SERVER_WRITEABLE:
  {
    // Fill the outbound pipe with frames of data
    assert(ThreadUtil::isDataStreamerThread());
    return jsonSession->write(wsi, context);
  }
  case LWS_CALLBACK_RECEIVE:
  {
    assert(ThreadUtil::isDataStreamerThread());
    if (len != 0)
    {
      string str((char const*)in, len);
      processCommand(str, jsonSession, context, wsi);
    }
    break;
  }
  default:
    // Unknown reason
    break;
  }

  return 0;
}
