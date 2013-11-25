#include "datastreamer.ih"

DataStreamer::DataStreamer(shared_ptr<Camera> camera)
  : d_image(),
    d_camera(camera),
    d_context(0),
    d_cameraSessions()
{
  d_port = Config::getStaticValue<int>("round-table.tcp-port");

  // We have three special protocols: HTTP-only, Camera and Control.
  // These are followed by N other protocols, one per type of state in the system

  unsigned protocolCount = 3 + AgentState::getInstance().stateTypeCount() + 1;

  d_protocols = new libwebsocket_protocols[protocolCount];

                   // name, callback, per-session-data-size, rx-buffer-size, no-buffer-all-partial-tx
  d_protocols[0] = { "http-only",        DataStreamer::_callback_http,    0,                     0, 0 };
  d_protocols[1] = { "camera-protocol",  DataStreamer::_callback_camera,  sizeof(CameraSession), 0, 0 };
  d_protocols[2] = { "control-protocol", DataStreamer::_callback_control, sizeof(JsonSession),   0, 0 };

  d_cameraProtocol = &d_protocols[1];
  d_controlProtocol = &d_protocols[2];

  // One protocol per state
  unsigned protocolIndex = 3;
  for (shared_ptr<StateTracker> stateTracker : AgentState::getInstance().getTrackers())
  {
    d_protocols[protocolIndex] = { stateTracker->name().c_str(), DataStreamer::_callback_state, 0, 0, 0 };
    stateTracker->websocketProtocol = &d_protocols[protocolIndex];
    protocolIndex++;
  }

  // Mark the end of the protocols
  d_protocols[protocolIndex] = { nullptr, nullptr, 0, 0, 0 };

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
    cerr << ccolor::error << "[DataStreamer::DataStreamer] libwebsocket context creation failed" << ccolor::reset << endl;

  //
  // Listen for state changes and publish them via websockets
  //
  AgentState::getInstance().updated.connect(
    [this](shared_ptr<StateTracker const> tracker) {
      assert(ThreadId::isThinkLoopThread());
      if (!d_hasWebSockets)
        return;
      libwebsocket_protocols* protocol = tracker->websocketProtocol;
      libwebsocket_callback_on_writable_all_protocol(protocol);
    }
  );

  Config::updated.connect(
    [this](SettingBase* setting)
    {
      assert(ThreadId::isThinkLoopThread());

      if (!d_hasWebSockets || d_controlSessions.size() == 0)
        return;

      StringBuffer buffer;
      Writer<StringBuffer> writer(buffer);

      writer.StartObject();
      {
        writer.String("type").String("update");
        writer.String("path").String(setting->getPath().c_str());
        writer.String("value");
        setting->writeJsonValue(writer);
      }
      writer.EndObject();

      auto bytes = JsonSession::createBytes(buffer);

      for (JsonSession* session : d_controlSessions)
        session->queue.push(bytes);

      libwebsocket_callback_on_writable_all_protocol(d_controlProtocol);
    }
  );
}
