#include "datastreamer.ih"

void DataStreamer::update()
{
  if (d_context == NULL)
    return;

  //
  // Only request sending images if we have a client who needs servicing
  //
  for (CameraSession* session : d_cameraSessions)
  {
    if (session->imgReady)
    {
      libwebsocket_callback_on_writable_all_protocol(d_cameraProtocol);
      break;
    }
  }

  for (ControlSession* session : d_controlSessions)
  {
    if (!session->queue.empty())
    {
      libwebsocket_callback_on_writable_all_protocol(d_controlProtocol);
      break;
    }
  }

  //
  // Process whatever else needs doing on the socket (new clients, etc)
  // This is normally very fast
  //
  libwebsocket_service(d_context, 0);
}
