#include "datastreamer.ih"

int DataStreamer::callback_timing(
  struct libwebsocket_context* /*context*/,
  struct libwebsocket *wsi,
  enum libwebsocket_callback_reasons reason,
  void* /*user*/,
  void* /*in*/,
  size_t /*len*/)
{
  // TODO review 512 size here... can apply a better cap
  unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512 + LWS_SEND_BUFFER_POST_PADDING];
  unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];

  if (reason == LWS_CALLBACK_SERVER_WRITEABLE)
  {
    AgentModel& agentModel = AgentModel::getInstance();

    int n = sprintf((char*)p, "%f|%f|%f|%f",
                    agentModel.lastImageCaptureTimeMillis,
                    agentModel.lastImageProcessTimeMillis,
                    agentModel.lastSubBoardReadTimeMillis,
                    agentModel.lastThinkCycleMillis);

    if (libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT) < 0)
    {
      lwsl_err("ERROR %d writing to socket\n", n);
      return 1;
    }
  }

  return 0;
}
