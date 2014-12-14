#include "datastreamer.ih"

int DataStreamer::callback_camera(
  libwebsocket_context* context,
  libwebsocket* wsi,
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
    return cameraSession->write();
  }
  default:
  {
    // Unknown reason
    break;
  }
  }

  return 0;
}
