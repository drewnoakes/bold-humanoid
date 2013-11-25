#include "datastreamer.ih"

int DataStreamer::callback_camera(
  struct libwebsocket_context* context,
  struct libwebsocket *wsi,
  enum libwebsocket_callback_reasons reason,
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
    // Can write to client
    if (cameraSession->imgReady)
    {
      prepareImageBytes(context, wsi, cameraSession);
    }
    break;
  }
  default:
    // Unknown reason
    break;
  }

  return 0;
}
