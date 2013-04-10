#include "datastreamer.ih"

int DataStreamer::callback_agent_model(
  struct libwebsocket_context* /*context*/,
  struct libwebsocket* wsi,
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
    BodyState& body = *AgentState::getInstance().body();

    int n = sprintf((char*)p, "%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f",
                    body.getCM730State().gyro.x(),
                    body.getCM730State().gyro.y(),
                    body.getCM730State().gyro.z(),
                    body.getCM730State().acc.x(),
                    body.getCM730State().acc.y(),
                    body.getCM730State().acc.z(),
                    body.getMX28State(1).presentPosition,
                    body.getMX28State(2).presentPosition,
                    body.getMX28State(3).presentPosition,
                    body.getMX28State(4).presentPosition,
                    body.getMX28State(5).presentPosition,
                    body.getMX28State(6).presentPosition,
                    body.getMX28State(7).presentPosition,
                    body.getMX28State(8).presentPosition,
                    body.getMX28State(9).presentPosition,
                    body.getMX28State(10).presentPosition,
                    body.getMX28State(11).presentPosition,
                    body.getMX28State(12).presentPosition,
                    body.getMX28State(13).presentPosition,
                    body.getMX28State(14).presentPosition,
                    body.getMX28State(15).presentPosition,
                    body.getMX28State(16).presentPosition,
                    body.getMX28State(17).presentPosition,
                    body.getMX28State(18).presentPosition,
                    body.getMX28State(19).presentPosition,
                    body.getMX28State(20).presentPosition);

    if (libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT) < 0)
    {
      lwsl_err("ERROR %d writing to socket\n", n);
      return 1;
    }
  }

  return 0;
}
