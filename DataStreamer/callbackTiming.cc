#include "datastreamer.ih"

int DataStreamer::callback_timing(
  struct libwebsocket_context* /*context*/,
  struct libwebsocket *wsi,
  enum libwebsocket_callback_reasons reason,
  void* /*user*/,
  void* /*in*/,
  size_t /*len*/)
{
  if (reason == LWS_CALLBACK_SERVER_WRITEABLE)
  {
    auto& debugger = Debugger::getInstance();
    std::vector<EventTiming> const& timings = debugger.getTimings();

    unsigned int bufLen = LWS_SEND_BUFFER_PRE_PADDING + (timings.size()*64) + LWS_SEND_BUFFER_POST_PADDING;
    unsigned char buf[bufLen];
    //memset(&buf, 0, bufLen);
    unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];

    int n = 0;
    for (EventTiming const& eventTiming : timings)
    {
      double timeSeconds = eventTiming.first;
      std::string eventName = eventTiming.second;
      n += sprintf((char*)(p + n), "%s=%f|", eventName.c_str(), timeSeconds);
    }

    if (libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT) < 0)
    {
      lwsl_err("ERROR %d writing to socket\n", n);
      return 1;
    }
  }

  return 0;
}
