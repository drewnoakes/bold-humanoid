#include "datastreamer.ih"

using namespace bold;
using namespace rapidjson;
using namespace std;

CameraSession::CameraSession(libwebsocket_context* context, libwebsocket *wsi)
  : imgReady(false),
    imgSending(false),
    imgJpgBuffer(make_unique<vector<uchar>>()),
    d_context(context),
    d_wsi(wsi)
{}

void CameraSession::notifyImageAvailable()
{
  ASSERT(ThreadUtil::isThinkLoopThread());

  if (!imgSending)
  {
    imgReady = true;
    libwebsocket_callback_on_writable(d_context, d_wsi);
  }
}
