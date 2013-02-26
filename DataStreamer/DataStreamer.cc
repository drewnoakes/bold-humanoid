#include "datastreamer.ih"

DataStreamer::DataStreamer(int port)
: d_port(port),
  d_gameStateUpdated(false)
{
  std::cout << "[DataStreamer::DataStreamer] creating on TCP port " << port << std::endl;
}

// static members

libwebsocket_protocols DataStreamer::d_protocols[] = {
  // name, callback, per-session-data-size
  { "http-only", DataStreamer::_callback_http, 0, NULL, 0 },
  { "timing-protocol", DataStreamer::_callback_timing, 0, NULL, 0 },
  { "game-state-protocol", DataStreamer::_callback_game_state, 0, NULL, 0 },
  { "agent-model-protocol", DataStreamer::_callback_agent_model, 0, NULL, 0 },
  { "camera-protocol", DataStreamer::_callback_camera, 0, NULL, 0 },
  { NULL, NULL, 0, NULL, 0 },
};
