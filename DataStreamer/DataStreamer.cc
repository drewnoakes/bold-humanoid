#include "datastreamer.ih"

DataStreamer::DataStreamer(minIni const& ini, shared_ptr<Camera> camera, std::shared_ptr<Debugger> debugger)
: d_controlsByIdByFamily(),
  d_image(),
  d_camera(camera),
  d_debugger(debugger),
  d_context(0),
  d_cameraSessions()
{
  cout << "[DataStreamer::DataStreamer] Starting" << endl;

  d_port = ini.geti("Debugger", "TcpPort", 8080);

  // We have three special protocols: HTTP-only, Camera and Control.
  // These are followed by N other protocols, one per type of state in the system

  unsigned protocolCount = 3 + AgentState::getInstance().stateTypeCount() + 1;

  d_protocols = new libwebsocket_protocols[protocolCount];

                   // name, callback, per-session-data-size, rx-buffer-size, owning-server, protocol-index
  d_protocols[0] = { "http-only", DataStreamer::_callback_http, 0, 0, NULL, 0 };
  d_protocols[1] = { "camera-protocol", DataStreamer::_callback_camera, sizeof(CameraSession), 0, NULL, 0 };
  d_protocols[2] = { "control-protocol", DataStreamer::_callback_control, 0, 0, NULL, 0 };

  d_cameraProtocol = &d_protocols[1];

  // One protocol per state
  unsigned protocolIndex = 3;
  for (shared_ptr<StateTracker> stateTracker : AgentState::getInstance().getTrackers())
  {
    d_protocols[protocolIndex] = { stateTracker->name().c_str(), DataStreamer::_callback_state, 0, 0, NULL, 0 };
    stateTracker->websocketProtocol = &d_protocols[protocolIndex];
    protocolIndex++;
  }

  // Mark the end of the protocols
  d_protocols[protocolIndex] = { NULL, NULL, 0, 0, NULL, 0 };

  //
  // Create the libwebsockets context object
  //
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
    cout << "[DataStreamer::DataStreamer] Listening on TCP port " << d_port << endl;

  //
  // Listen for state changes, and publish them via websockets
  //
  AgentState::getInstance().updated.connect(
    [](shared_ptr<StateTracker const> tracker) {
      libwebsocket_protocols* protocol = tracker->websocketProtocol;
      libwebsocket_callback_on_writable_all_protocol(protocol);
    }
  );
}
