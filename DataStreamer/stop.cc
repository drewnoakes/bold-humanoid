#include "datastreamer.ih"

void DataStreamer::stop()
{
  if (d_isStopRequested)
  {
    log::error("DataStreamer::stop") << "Stop has already been requested";
    return;
  }

  d_isStopRequested = true;

  d_thread.join();
}
