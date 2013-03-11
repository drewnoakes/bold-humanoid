#include "datastreamer.ih"

void DataStreamer::update()
{
  if (d_context == NULL)
    return;

  //
  // Only register for writing to sockets where we have changes to report
  //
  if (d_gameStateUpdated)
  {
    d_gameStateUpdated = false;
    libwebsocket_callback_on_writable_all_protocol(&d_protocols[Protocol::GAME_STATE]);
  }

  if (d_agentModelUpdated)
  {
    d_agentModelUpdated = false;
    libwebsocket_callback_on_writable_all_protocol(&d_protocols[Protocol::AGENT_MODEL]);
  }

  //
  // We always have new timing data available
  //
  libwebsocket_callback_on_writable_all_protocol(&d_protocols[Protocol::TIMING]);

  //
  // Only request sending images if we have a client who needs servicing
  //
  for (CameraSession* cameraSession : d_cameraSessions)
  {
    if (!cameraSession->hasSentStateAndOptions || cameraSession->imgReady)
    {
      libwebsocket_callback_on_writable_all_protocol(&d_protocols[Protocol::CAMERA]);
      break;
    }
  }

  //
  // Process whatever else needs doing on the socket (new clients, etc)
  // This is normally very fast
  //
  libwebsocket_service(d_context, 0);
}
