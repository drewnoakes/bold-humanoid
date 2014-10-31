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
    // Process whatever needs doing on the web socket (new clients, writing, receiving, etc)
    //
    libwebsocket_service(d_context, 10);
  }

  if (d_context)
    libwebsocket_context_destroy(d_context);
}
