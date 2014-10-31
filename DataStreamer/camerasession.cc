#include "datastreamer.ih"

using namespace bold;
using namespace rapidjson;
using namespace std;

CameraSession::CameraSession(libwebsocket_protocols* cameraProtocol)
  : d_cameraProtocol(cameraProtocol),
    imgReady(false),
    imgSending(false),
    imgJpgBuffer(make_unique<vector<uchar>>())
{}

void CameraSession::notifyImageAvailable()
{
  if (!imgSending)
  {
    imgReady = true;
    libwebsocket_callback_on_writable_all_protocol(d_cameraProtocol);
  }
}
