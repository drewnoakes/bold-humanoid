#include "datastreamer.ih"

DataStreamer::DataStreamer(int port)
: d_gameStateUpdated(false),
  d_agentModelUpdated(false),
  d_image(),
  d_imageType(ImageType::RGB),
  d_streamFramePeriod(10),
  d_shouldDrawBlobs(true),
  d_shouldDrawLineDots(false),
  d_shouldDrawExpectedLines(true),
  d_shouldDrawObservedLines(true),
  d_camera(0),
  d_cameraSessions(),
  d_controlsByIdByFamily(),
  d_port(port),
  d_context(0)
{}

// static members

libwebsocket_protocols DataStreamer::d_protocols[] = {
  // name, callback, per-session-data-size, rx-buffer-size, owning-server, protocol-index
  { "http-only", DataStreamer::_callback_http, 0, 0, NULL, 0 },
  { "timing-protocol", DataStreamer::_callback_timing, 0, 0, NULL, 0 },
  { "game-state-protocol", DataStreamer::_callback_game_state, 0, 0, NULL, 0 },
  { "agent-model-protocol", DataStreamer::_callback_agent_model, 0, 0, NULL, 0 },
  { "camera-protocol", DataStreamer::_callback_camera, sizeof(CameraSession), 0, NULL, 0 },
  { NULL, NULL, 0, 0, NULL, 0 },
};
