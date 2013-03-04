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
    cameraSession->labelSelection = d_imgStreams.begin()->first;
    d_cameraSessions.push_back(cameraSession);
    break;

  case LWS_CALLBACK_SERVER_WRITEABLE:
    // Can write to client
    switch (cameraSession->state)
    {
    case CameraSession::SEND_CONTROLS:
      sendCameraControls(wsi);
      cameraSession->state = CameraSession::SEND_IMG_TAGS;
      break;

    case CameraSession::SEND_IMG_TAGS:
      cameraSession->state = CameraSession::SEND_IMAGE;
      break;

    case CameraSession::SEND_IMAGE:
      if (cameraSession->imgReady)
        sendImage(wsi, d_imgStreams[cameraSession->labelSelection]);
      break;
    }

  case LWS_CALLBACK_RECEIVE:
    if (len > 0)
    {
      string str((char const*)in, len);
      istringstream controlIn(str);
      unsigned controlId, controlVal;
      controlIn >> controlId >> controlVal;
      cout << "[DataStreamer::callback_camera] client receive: " << controlId << " <- " << controlVal << endl;
      auto controls = d_camera->getControls();
      auto control = find_if(controls.begin(), controls.end(),\
                             [controlId](Camera::Control const& c)
                             {
                               return c.id == controlId;
                             });
      if (control != controls.end())
        control->setValue(controlVal);
    }
    break;
  }

  libwebsocket_callback_on_writable_all_protocol(&d_protocols[Protocol::CAMERA]);

  return 0;
}

