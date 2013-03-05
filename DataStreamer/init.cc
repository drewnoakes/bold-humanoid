#include "datastreamer.ih"

void DataStreamer::init()
{
  std::cout << "[DataStreamer:init] Starting" << std::endl;

  lws_context_creation_info contextInfo;
  memset(&contextInfo, 0, sizeof(contextInfo));
  contextInfo.port = d_port;
  contextInfo.protocols = d_protocols;
  contextInfo.gid = contextInfo.uid = -1;
  contextInfo.user = this;

  d_context = libwebsocket_create_context(&contextInfo);

  if (d_context == NULL)
    lwsl_err("libwebsocket init failed\n");
  else
    std::cout << "[DataStreamer:init] libwebsocket_context created" << std::endl;

  GameState::getInstance().updated.connect([this]{ d_gameStateUpdated = true; });
  AgentModel::getInstance().updated.connect([this]{ d_agentModelUpdated = true; });

  std::cout << "[DataStreamer:init] Done" << std::endl;
}
