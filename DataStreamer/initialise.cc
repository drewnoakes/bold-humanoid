#include "datastreamer.ih"

void DataStreamer::initialise(minIni const& ini)
{
  std::cout << "[DataStreamer:initialise] Starting" << std::endl;

  d_streamFramePeriod = ini.geti("Debugger", "BroadcastFramePeriod", 5);

  lws_context_creation_info contextInfo;
  memset(&contextInfo, 0, sizeof(contextInfo));
  contextInfo.port = d_port;
  contextInfo.protocols = d_protocols;
  contextInfo.gid = contextInfo.uid = -1;
  contextInfo.user = this;

  d_context = libwebsocket_create_context(&contextInfo);

  if (d_context == NULL)
    lwsl_err("libwebsocket context creation failed\n");
  else
    std::cout << "[DataStreamer:initialise] Listening on TCP port " << d_port << std::endl;

  GameState::getInstance().updated.connect([this]{ d_gameStateUpdated = true; });
  AgentModel::getInstance().updated.connect([this]{ d_agentModelUpdated = true; });

  // TODO split into vision and head controls
  registerControls("debug", getDebugControls());
}
