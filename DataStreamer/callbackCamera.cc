#include "datastreamer.ih"

int DataStreamer::callback_camera(
  libwebsocket_context* context,
  libwebsocket *wsi,
  libwebsocket_callback_reasons reason,
  void* session,
  void* in,
  size_t len)
{
  assert(ThreadId::isThinkLoopThread());

  CameraSession* cameraSession = reinterpret_cast<CameraSession*>(session);

  switch (reason)
  {
  case LWS_CALLBACK_ESTABLISHED:
  {
    // New client connected; initialize session
    cameraSession->imgReady = false;
    cameraSession->imgSending = false;
    cameraSession->imgJpgBuffer = unique_ptr<vector<uchar>>(new vector<uchar>());
    d_cameraSessions.push_back(cameraSession);
    break;
  }
  case LWS_CALLBACK_CLOSED:
  {
    // Client disconnected
    d_cameraSessions.erase(find(d_cameraSessions.begin(), d_cameraSessions.end(), cameraSession));
    break;
  }
  case LWS_CALLBACK_SERVER_WRITEABLE:
  {
    if (!cameraSession->imgReady)
      break;

    if (!cameraSession->imgSending)
    {
      // Encode JPEG
      cv::imencode(".jpg", d_image, *(cameraSession->imgJpgBuffer));

      cameraSession->imgSending = true;
      cameraSession->bytesSent = 0;
    }

    // Fill the outbound pipe with frames of data
    while (!lws_send_pipe_choked(wsi))
    {
      uint totalSize = cameraSession->imgJpgBuffer->size();
      uchar* start = cameraSession->imgJpgBuffer->data() + cameraSession->bytesSent;

      uint remainingSize = totalSize - cameraSession->bytesSent;
      uint frameSize = min(2048u, remainingSize);
      uchar buf[LWS_SEND_BUFFER_PRE_PADDING + frameSize + LWS_SEND_BUFFER_POST_PADDING];
      uchar *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];

      memcpy(p, start, frameSize);

      int writeMode = cameraSession->bytesSent == 0
        ? LWS_WRITE_BINARY
        : LWS_WRITE_CONTINUATION;

      if (frameSize != remainingSize)
        writeMode |= LWS_WRITE_NO_FIN;

      int res = libwebsocket_write(wsi, p, frameSize, (libwebsocket_write_protocol)writeMode);

      if (res < 0)
      {
        lwsl_err("ERROR %d writing to socket (image)\n", res);
        return 1;
      }

      cameraSession->bytesSent += frameSize;

      if (cameraSession->bytesSent == totalSize)
      {
        // Done sending
        cameraSession->imgReady = false;
        cameraSession->imgSending = false;
        cameraSession->bytesSent = 0;
        return 0;
      }
    }

    // Queue for more writing later on if we still have data remaining
    if (cameraSession->imgSending)
      libwebsocket_callback_on_writable(context, wsi);

    break;
  }
  default:
    // Unknown reason
    break;
  }

  return 0;
}
