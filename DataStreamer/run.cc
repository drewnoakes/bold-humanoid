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
    libwebsocket_service(d_context, 0);

    // TODO would using some kind of signal between threads be more efficient than sleeping for a fixed period?
    static Setting<int>* dataStreamerSleepMicroseconds = Config::getSetting<int>("data-streamer.thread-sleep-microseconds");
    this_thread::sleep_for(chrono::microseconds(dataStreamerSleepMicroseconds->getValue()));
  }
}
