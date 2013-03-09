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
    AgentModel& agentModel = AgentModel::getInstance();

    int n = sprintf((char*)p, "%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f",
                    agentModel.cm730State.gyro.x(),
                    agentModel.cm730State.gyro.y(),
                    agentModel.cm730State.gyro.z(),
                    agentModel.cm730State.acc.x(),
                    agentModel.cm730State.acc.y(),
                    agentModel.cm730State.acc.z(),
                    agentModel.mx28States[1].presentPosition,
                    agentModel.mx28States[2].presentPosition,
                    agentModel.mx28States[3].presentPosition,
                    agentModel.mx28States[4].presentPosition,
                    agentModel.mx28States[5].presentPosition,
                    agentModel.mx28States[6].presentPosition,
                    agentModel.mx28States[7].presentPosition,
                    agentModel.mx28States[8].presentPosition,
                    agentModel.mx28States[9].presentPosition,
                    agentModel.mx28States[10].presentPosition,
                    agentModel.mx28States[11].presentPosition,
                    agentModel.mx28States[12].presentPosition,
                    agentModel.mx28States[13].presentPosition,
                    agentModel.mx28States[14].presentPosition,
                    agentModel.mx28States[15].presentPosition,
                    agentModel.mx28States[16].presentPosition,
                    agentModel.mx28States[17].presentPosition,
                    agentModel.mx28States[18].presentPosition,
                    agentModel.mx28States[19].presentPosition,
                    agentModel.mx28States[20].presentPosition);

    if (libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT) < 0)
    {
      lwsl_err("ERROR %d writing to socket\n", n);
      return 1;
    }
  }

  return 0;
}
