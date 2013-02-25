#include "datastreamer.ih"

void DataStreamer::close()
{
  if (d_context == NULL)
    return;

  libwebsocket_context_destroy(d_context);
}
