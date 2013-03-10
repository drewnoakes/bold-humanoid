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
    if (len == 0)
      break;

    string str((char const*)in, len);

    // Parse JSON
    rapidjson::Document d;
    d.Parse<0>(str.c_str());

    if (d.HasParseError())
    {
      cerr << "[DataStreamer::callbackCamera] Error parsing JSON request" << endl;
      break;
    }

    if (!d.HasString("command"))
    {
      cerr << "[DataStreamer::callbackCamera] No command specified in received message" << endl;
      break;
    }

    string command(d["command"].GetString());
    if (command == "setControl")
    {
      unsigned controlId;
      unsigned controlVal;
      if (!d.TryGetUintMember("id", &controlId) || !d.TryGetUintMember("val", &controlVal))
      {
        cerr << "[DataStreamer::callbackCamera] Invalid setControl command" << endl;
        break;
      }

      auto control = d_camera->getControl(controlId);
      if (control.hasValue())
        control.value()->setValue(controlVal);
    }
    else if (command == "selectStream")
    {
      unsigned streamId;
      if (!d.TryGetUintMember("id", &streamId))
      {
        cerr << "[DataStreamer::callbackCamera] Invalid selectStream command" << endl;
        break;
      }

      cameraSession->streamSelection = streamId;
    }
    else if (command == "controlHead")
    {
      if (!d.HasString("action"))
      {
        cerr << "[DataStreamer::callbackCamera] Invalid controlHead command" << endl;
        break;
      }
      string action(d["action"].GetString());

      Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);

      if (action == "<")
        Head::GetInstance()->MoveByAngleOffset(5,0);
      else if (action == ">")
        Head::GetInstance()->MoveByAngleOffset(-5,0);
      else if (action == "^")
        Head::GetInstance()->MoveByAngleOffset(0,5);
      else if (action == "v")
        Head::GetInstance()->MoveByAngleOffset(0,-5);
    }

    break;
  }

  return 0;
}

