#include "datastreamer.ih"

int DataStreamer::callback_camera(
  libwebsocket_context* context,
  libwebsocket *wsi,
  libwebsocket_callback_reasons reason,
  void* session,
  void*,
  size_t)
{
  CameraSession* cameraSession = reinterpret_cast<CameraSession*>(session);

  switch (reason)
  {
  case LWS_CALLBACK_ESTABLISHED:
  {
    ASSERT(ThreadUtil::isDataStreamerThread());

    log::verbose("camera-protocol") << "Client connected";

    // New client connected; initialize session
    new (cameraSession) CameraSession(context, wsi);

    lock_guard<mutex> guard(d_cameraSessionsMutex);
    d_cameraSessions.push_back(cameraSession);

    if (d_cameraSessions.size() == 1)
      hasClientChanged("camera-protocol", true);

    break;
  }
  case LWS_CALLBACK_CLOSED:
  {
    // Client disconnected
    ASSERT(ThreadUtil::isDataStreamerThread());

    log::verbose("camera-protocol") << "Client disconnecting";

    lock_guard<mutex> guard(d_cameraSessionsMutex);
    d_cameraSessions.erase(find(d_cameraSessions.begin(), d_cameraSessions.end(), cameraSession));

    if (d_cameraSessions.size() == 0)
      hasClientChanged("camera-protocol", false);

    cameraSession->~CameraSession();

    break;
  }
  case LWS_CALLBACK_SERVER_WRITEABLE:
  {
    ASSERT(ThreadUtil::isDataStreamerThread());

    if (!cameraSession->imgReady)
      break;

    if (!cameraSession->imgSending)
    {
      // Encode JPEG
      cv::imencode(d_imageEncoding, d_image, *(cameraSession->imgJpgBuffer));

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

      // TODO avoid this copy here by juggling the post padding
      memcpy(p, start, frameSize);

      int writeMode = cameraSession->bytesSent == 0
        ? LWS_WRITE_BINARY
        : LWS_WRITE_CONTINUATION;

      if (frameSize != remainingSize)
        writeMode |= LWS_WRITE_NO_FIN;

      int res = libwebsocket_write(wsi, p, frameSize, (libwebsocket_write_protocol)writeMode);

      if (res < 0)
      {
        log::error("callback_camera") << "Error " << res << " writing to socket (image)";
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
