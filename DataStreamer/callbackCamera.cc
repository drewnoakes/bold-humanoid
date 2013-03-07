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
    cameraSession->streamSelection = 0;
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
    case CameraSession::SEND_CONTROLS:
      sendCameraControls(wsi);
      cameraSession->state = CameraSession::SEND_IMG_TAGS;
      break;

    case CameraSession::SEND_IMG_TAGS:
      sendStreamLabels(wsi);
      cameraSession->state = CameraSession::SEND_IMAGE;
      break;

    case CameraSession::SEND_IMAGE:
      if (cameraSession->imgReady)
      {
        sendImage(wsi, cameraSession);
      }
      break;
    }

  case LWS_CALLBACK_RECEIVE:
    if (len > 0)
    {
      string str((char const*)in, len);

      // Parse JSON
      rapidjson::Document d;
      d.Parse<0>(str.c_str());
      if (d.HasMember("command"))
      {
	string command(d["command"].GetString());
	if (command == "setControl")
	{
	  unsigned controlId = d["id"].GetUint();
	  unsigned controlVal = d["val"].GetUint();

	  auto controls = d_camera->getControls();
	  auto control = find_if(controls.begin(), controls.end(),	\
				 [controlId](Camera::Control const& c)
				 {
				   return c.id == controlId;
				 });
	  if (control != controls.end())
	    control->setValue(controlVal);
	}
	else if (command == "selectStream")
	{
	  unsigned streamId = d["id"].GetUint();
	  cameraSession->streamSelection = streamId;
	}
      }

    }
    break;
  }

  return 0;
}

