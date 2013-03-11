#include "datastreamer.ih"

int DataStreamer::callback_camera(
  struct libwebsocket_context* /*context*/,
  struct libwebsocket *wsi,
  enum libwebsocket_callback_reasons reason,
  void* session,
  void* in,
  size_t len)
{
  CameraSession* cameraSession = reinterpret_cast<CameraSession*>(session);

  switch (reason)
  {
  case LWS_CALLBACK_ESTABLISHED:
    // New client connected; initialize session
    memset(cameraSession, 0, sizeof(CameraSession));
    d_cameraSessions.push_back(cameraSession);
    break;

  case LWS_CALLBACK_CLOSED:
    // Client disconnected
    d_cameraSessions.erase(find(d_cameraSessions.begin(), d_cameraSessions.end(), cameraSession));
    break;

  case LWS_CALLBACK_SERVER_WRITEABLE:
    // Can write to client
    switch (cameraSession->state)
    {
    case CameraSession::State::SEND_CONTROLS:
      sendCameraControls(wsi);
      cameraSession->state = CameraSession::State::SEND_IMG_TYPES;
      break;

    case CameraSession::State::SEND_IMG_TYPES:
      sendImageTypes(wsi);
      cameraSession->state = CameraSession::State::SEND_IMAGE;
      break;

    case CameraSession::State::SEND_IMAGE:
      if (cameraSession->imgReady)
      {
        sendImageBytes(wsi, cameraSession);
      }
      break;
    }
    break;

  case LWS_CALLBACK_RECEIVE:
    if (len == 0)
      break;

    string str((char const*)in, len);

    processCameraCommand(str);

    break;
  }

  return 0;
}

