#include "datastreamer.ih"

void DataStreamer::prepareImageBytes(libwebsocket_context* context, libwebsocket* wsi, CameraSession* session) const
{
  assert(session->imgReady);

  if (!session->imgSending)
  {
    //
    // Encode JPEG
    //
    cv::imencode(".jpg", d_image, *(session->imgJpgBuffer));

    //
    // Send prefix with number of bytes, so the client can merge the frames
    //
    size_t jpegByteCount = session->imgJpgBuffer->size();

    ostringstream prefOut;
    prefOut << jpegByteCount;
    string pref = prefOut.str();

    unsigned char prefBuf[LWS_SEND_BUFFER_PRE_PADDING + pref.size() + LWS_SEND_BUFFER_POST_PADDING];
    unsigned char *p = &prefBuf[LWS_SEND_BUFFER_PRE_PADDING];

    memcpy(p, pref.c_str(), pref.size());
    int res = libwebsocket_write(wsi, p, pref.size(), LWS_WRITE_TEXT);
    if (res < 0)
    {
      // TODO: make this method return error code
      lwsl_err("ERROR %d writing to socket (prefix)\n", res);
      return;
    }

    session->imgSending = true;
    session->imgBytesSent = 0;
  }

  //
  // Fill the outbound pipe with frames of image data
  //
  while (!lws_send_pipe_choked(wsi))
  {
    size_t jpgSize = session->imgJpgBuffer->size();
    uchar* jpgP = session->imgJpgBuffer->data() + session->imgBytesSent;

    int tosend = jpgSize - session->imgBytesSent;
    unsigned bufsize = min(2048, tosend);
    unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + bufsize + LWS_SEND_BUFFER_POST_PADDING];
    unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];

    memcpy(p, jpgP, bufsize);

    int res = libwebsocket_write(wsi, p, bufsize, LWS_WRITE_BINARY);
    if (res < 0)
    {
      // TODO: make this method return error code
      lwsl_err("ERROR %d writing to socket (image) (%d/%d)\n", res, tosend, session->imgJpgBuffer->size());
      return;
    }
    session->imgBytesSent += bufsize;

    if (session->imgBytesSent == jpgSize)
    {
      // Done sending
      session->imgReady = false;
      session->imgSending = false;
      session->imgBytesSent = 0;
      return;
    }
  }

  //
  // Queue for more writing if we still have image remaining
  //
  if (session->imgSending)
    libwebsocket_callback_on_writable(context, wsi);
}
