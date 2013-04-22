#include "datastreamer.ih"

void DataStreamer::update()
{
  if (d_context == NULL)
    return;

  //
  // We always have new timing data available
  //
  libwebsocket_callback_on_writable_all_protocol(d_timingProtocol);

  //
  // Only request sending images if we have a client who needs servicing
  //
  for (CameraSession* cameraSession : d_cameraSessions)
  {
    if (cameraSession->imgReady)
    {
      libwebsocket_callback_on_writable_all_protocol(d_cameraProtocol);
      break;
    }
  }

  //
  // Process whatever else needs doing on the socket (new clients, etc)
  // This is normally very fast
  //
  libwebsocket_service(d_context, 0);
}
