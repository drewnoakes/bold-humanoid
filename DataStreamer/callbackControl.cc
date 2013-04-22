#include "datastreamer.ih"

int DataStreamer::callback_control(
  struct libwebsocket_context* context,
  struct libwebsocket *wsi,
  enum libwebsocket_callback_reasons reason,
  void* /*session*/,
  void* in,
  size_t len)
{
  switch (reason)
  {
  case LWS_CALLBACK_ESTABLISHED:
  {
    libwebsocket_callback_on_writable(context, wsi);
    return 0;
  }
  case LWS_CALLBACK_SERVER_WRITEABLE:
  {
    sendCameraControls(wsi);
    break;
  }
  case LWS_CALLBACK_RECEIVE:
  {
    if (len != 0)
    {
      string str((char const*)in, len);
      processCommand(str);
    }
    break;
  }
  default:
    // Unknown reason
    break;
  }

  return 0;
}
