#include "datastreamer.ih"

int DataStreamer::callback_control(
  struct libwebsocket_context* context,
  struct libwebsocket *wsi,
  enum libwebsocket_callback_reasons reason,
  void* session,
  void* in,
  size_t len)
{
  assert(ThreadId::isThinkLoopThread());

  JsonSession* jsonSession = reinterpret_cast<JsonSession*>(session);

  switch (reason)
  {
  case LWS_CALLBACK_ESTABLISHED:
  {
    // New client connected; initialize session
    jsonSession->bytesSent = 0;
    jsonSession->queue = queue<shared_ptr<vector<uchar> const>>();
    d_controlSessions.push_back(jsonSession);
    jsonSession->queue.push(prepareControlSyncBytes());
    libwebsocket_callback_on_writable(context, wsi);
    return 0;
  }
  case LWS_CALLBACK_CLOSED:
  {
    // Client disconnected
    d_controlSessions.erase(find(d_controlSessions.begin(), d_controlSessions.end(), jsonSession));
    break;
  }
  case LWS_CALLBACK_SERVER_WRITEABLE:
  {
    // Fill the outbound pipe with frames of data
    while (!lws_send_pipe_choked(wsi) && !jsonSession->queue.empty())
    {
      shared_ptr<vector<uchar> const> const& str = jsonSession->queue.front();
      assert(str);
      uint totalSize = str.get()->size();

      assert(jsonSession->bytesSent < totalSize);

      const uchar* start = str.get()->data() + jsonSession->bytesSent;

      uint remainingSize = totalSize - jsonSession->bytesSent;
      uint frameSize = min(2048u, remainingSize);
      uchar frameBuffer[LWS_SEND_BUFFER_PRE_PADDING + frameSize + LWS_SEND_BUFFER_POST_PADDING];
      uchar *p = &frameBuffer[LWS_SEND_BUFFER_PRE_PADDING];

      memcpy(p, start, frameSize);

      int writeMode = jsonSession->bytesSent == 0
        ? LWS_WRITE_TEXT
        : LWS_WRITE_CONTINUATION;

      if (frameSize != remainingSize)
        writeMode |= LWS_WRITE_NO_FIN;

      int res = libwebsocket_write(wsi, p, frameSize, (libwebsocket_write_protocol)writeMode);

      if (res < 0)
      {
        lwsl_err("ERROR %d writing to socket (control)\n", res);
        return 1;
      }

      jsonSession->bytesSent += frameSize;

      if (jsonSession->bytesSent == totalSize)
      {
        // Done sending this queue item, so ditch it, reset and loop around again
        jsonSession->queue.pop();
        jsonSession->bytesSent = 0;
      }
    }

    // Queue for more writing later on if we still have data remaining
    if (!jsonSession->queue.empty())
      libwebsocket_callback_on_writable(context, wsi);

    break;
  }
  case LWS_CALLBACK_RECEIVE:
  {
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
