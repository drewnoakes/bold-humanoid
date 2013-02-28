#include "datastreamer.ih"

void DataStreamer::sendCameraControls(libwebsocket* wsi)
{
  cout << "[DataStreamer::sendCameraControls] start" << endl;

  auto controls = d_camera->getControls();

  cout << "[DataStreamer::sendCameraControls] nr of controls: " << controls.size() << endl;
  ostringstream controlsOut;
  controlsOut << "[";
  bool fst = true;
  for (auto& control : controls)
  {
    if (!fst)
      controlsOut << ",";
    fst = false;

    controlsOut << "{" <<
      "\"id\":" << control.id <<
      ",\"type\":" << control.type <<
      ",\"name\":\"" << control.name << "\"" <<
      ",\"minimum\":" << control.minimum <<
      ",\"maximum\":" << control.maximum <<
      ",\"step\":" << control.step <<
      ",\"value\":" << control.getValue();

    if (control.type == Camera::CT_MENU)
    {
      controlsOut << ",\"menuItems\":[";
      fst = true;
      for (auto& menuItem : control.menuItems)
      {
        if (!fst)
          controlsOut << ",";
        fst = false;
        controlsOut << "{" <<
          "\"id\":" << menuItem.id <<
          ",\"index\":" << menuItem.index <<
          ",\"name\":\"" << menuItem.name << "\"" <<
          "}";
      }
      controlsOut << "]";
    }
    controlsOut << "}";

  }
  controlsOut << "]";

  string controlsStr = controlsOut.str();
  cout << "[DataStreamer::sendCameraControls] sending: " << controlsStr << endl;

  unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING +
                    controlsStr.size() +
                    LWS_SEND_BUFFER_POST_PADDING];
  unsigned char* p = buf + LWS_SEND_BUFFER_POST_PADDING;
  memcpy(p, controlsStr.c_str(), controlsStr.size());

  int res = libwebsocket_write(wsi, p, controlsStr.size(), LWS_WRITE_TEXT);
}
