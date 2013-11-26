#include "datastreamer.ih"

void DataStreamer::prepareImageBytes(libwebsocket_context* context, libwebsocket* wsi, CameraSession* session) const
{
  assert(session->imgReady);

  if (!session->imgSending)
  {
    // Encode JPEG
    cv::imencode(".jpg", d_image, *(session->imgJpgBuffer));

    session->imgSending = true;
    session->bytesSent = 0;
  }

  // Fill the outbound pipe with frames of data
  while (!lws_send_pipe_choked(wsi))
  {
    uint totalSize = session->imgJpgBuffer->size();
    uchar* start = session->imgJpgBuffer->data() + session->bytesSent;

    uint remainingSize = totalSize - session->bytesSent;
    uint frameSize = min(2048u, remainingSize);
    uchar buf[LWS_SEND_BUFFER_PRE_PADDING + frameSize + LWS_SEND_BUFFER_POST_PADDING];
    uchar *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];

    memcpy(p, start, frameSize);

    int writeMode = session->bytesSent == 0
      ? LWS_WRITE_BINARY
      : LWS_WRITE_CONTINUATION;

    if (frameSize != remainingSize)
      writeMode |= LWS_WRITE_NO_FIN;

    int res = libwebsocket_write(wsi, p, frameSize, (libwebsocket_write_protocol)writeMode);

    if (res < 0)
    {
      // TODO make this method return error code
      lwsl_err("ERROR %d writing to socket (image)\n", res);
      return;
    }

    session->bytesSent += frameSize;

    if (session->bytesSent == totalSize)
    {
      // Done sending
      session->imgReady = false;
      session->imgSending = false;
      session->bytesSent = 0;
      return;
    }
  }

    // Queue for more writing later on if we still have data remaining
  if (session->imgSending)
    libwebsocket_callback_on_writable(context, wsi);
}
