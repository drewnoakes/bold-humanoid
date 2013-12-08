#include "datastreamer.ih"

using namespace bold;
using namespace std;

void JsonSession::initialise()
{
  bytesSent = 0;
  queue = std::queue<shared_ptr<vector<uchar> const>>();
}

int JsonSession::write(libwebsocket* wsi, libwebsocket_context* context)
{
  // Fill the outbound pipe with frames of data
  while (!lws_send_pipe_choked(wsi) && !queue.empty())
  {
    shared_ptr<vector<uchar> const> const& str = queue.front();
    assert(str);
    uint totalSize = str.get()->size();

    assert(bytesSent < totalSize);

    const uchar* start = str.get()->data() + bytesSent;

    uint remainingSize = totalSize - bytesSent;
    uint frameSize = min(2048u, remainingSize);
    uchar frameBuffer[LWS_SEND_BUFFER_PRE_PADDING + frameSize + LWS_SEND_BUFFER_POST_PADDING];
    uchar *p = &frameBuffer[LWS_SEND_BUFFER_PRE_PADDING];

    memcpy(p, start, frameSize);

    int writeMode = bytesSent == 0
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

    bytesSent += frameSize;

    if (bytesSent == totalSize)
    {
      // Done sending this queue item, so ditch it, reset and loop around again
      queue.pop();
      bytesSent = 0;
    }
  }

  // Queue for more writing later on if we still have data remaining
  if (!queue.empty())
    libwebsocket_callback_on_writable(context, wsi);

  return 0;
}
