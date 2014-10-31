#include "datastreamer.ih"

void DataStreamer::run()
{
  if (!d_context)
  {
    log::error("DataStreamer::run") << "LWS context has not been established";
    throw runtime_error("LWS context has not been established");
  }

  ThreadUtil::setThreadId(ThreadId::DataStreamer);

  while (!d_isStopRequested)
  {
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

    //
    // Process whatever else needs doing on the socket (new clients, etc)
    // This is normally very fast
    //
    libwebsocket_service(d_context, 10);
  }

  if (d_context)
    libwebsocket_context_destroy(d_context);
}
