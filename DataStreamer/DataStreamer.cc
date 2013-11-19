#include "datastreamer.ih"

DataStreamer::DataStreamer(shared_ptr<Camera> camera)
  : d_controlsByIdByFamily(),
    d_image(),
    d_camera(camera),
    d_context(0),
    d_cameraSessions()
{
  cout << "[DataStreamer::DataStreamer] Starting" << endl;

  d_port = Config::getStaticValue<int>("round-table.tcp-port");

  // We have three special protocols: HTTP-only, Camera and Control.
  // These are followed by N other protocols, one per type of state in the system

  unsigned protocolCount = 3 + AgentState::getInstance().stateTypeCount() + 1;

  d_protocols = new libwebsocket_protocols[protocolCount];

                   // name, callback, per-session-data-size, rx-buffer-size, owning-server, protocol-index
  d_protocols[0] = { "http-only", DataStreamer::_callback_http, 0, 0, NULL, 0 };
  d_protocols[1] = { "camera-protocol", DataStreamer::_callback_camera, sizeof(CameraSession), 0, NULL, 0 };
  // TODO this 16kB hack gets around the outbound buffer size problem -- a better solution is to write in a loop until the pipe is choked
  // see http://ml.libwebsockets.org/pipermail/libwebsockets/2013-April/000432.html
  d_protocols[2] = { "control-protocol", DataStreamer::_callback_control, 0, 16*1024, NULL, 0 };

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

  d_hasWebSockets = d_context != nullptr;

  if (d_hasWebSockets)
    cout << "[DataStreamer::DataStreamer] Listening on TCP port " << d_port << endl;
  else
    cerr << "[DataStreamer::DataStreamer] libwebsocket context creation failed" << endl;

  //
  // Listen for state changes and publish them via websockets
  //
  AgentState::getInstance().updated.connect(
    [this](shared_ptr<StateTracker const> tracker) {
      if (!d_hasWebSockets)
        return;
      libwebsocket_protocols* protocol = tracker->websocketProtocol;
      libwebsocket_callback_on_writable_all_protocol(protocol);
    }
  );
}
