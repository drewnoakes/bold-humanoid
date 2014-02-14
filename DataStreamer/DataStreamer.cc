#include "datastreamer.ih"

DataStreamer::DataStreamer(shared_ptr<Camera> camera)
  : d_port(Config::getStaticValue<int>("round-table.tcp-port")),
    d_camera(camera),
    d_image(),
    d_context(0),
    d_cameraSessions(),
    d_isStopRequested(false),
    d_thread()
{
  // We have three special protocols: HTTP-only, Camera and Control.
  // These are followed by N other protocols, one per type of state in the system

  unsigned protocolCount = 3 + AgentState::stateTypeCount() + 1;

  d_protocols = new libwebsocket_protocols[protocolCount];

                   // name, callback, per-session-data-size, rx-buffer-size, no-buffer-all-partial-tx
  d_protocols[0] = { "http-only",        DataStreamer::_callback_http,    0,                     0, 0 };
  d_protocols[1] = { "camera-protocol",  DataStreamer::_callback_camera,  sizeof(CameraSession), 0, 0 };
  d_protocols[2] = { "control-protocol", DataStreamer::_callback_control, sizeof(JsonSession),   0, 0 };

  d_cameraProtocol = &d_protocols[1];
  d_controlProtocol = &d_protocols[2];

  // One protocol per state
  unsigned protocolIndex = 3;
  for (shared_ptr<StateTracker> stateTracker : AgentState::getTrackers())
  {
    d_protocols[protocolIndex] = { stateTracker->name().c_str(), DataStreamer::_callback_state, sizeof(JsonSession), 0, 0 };
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

  bool hasWebSockets = d_context != nullptr;

  if (hasWebSockets)
    log::info("DataStreamer::DataStreamer") << "Listening on TCP port " << d_port;
  else
    log::error("DataStreamer::DataStreamer") << "libwebsocket context creation failed";

  if (hasWebSockets)
  {
    // Listen for StateObject changes and publish them via websockets
    AgentState::updated.connect(
      [this](shared_ptr<StateTracker const> tracker)
      {
        // NOTE we may be writing from one thread, while another is dealing with
        // a client connection (which modifies d_stateSessions), or sending data
        // (which modifies the JsonSession queue.)

        std::lock_guard<std::mutex> guard(d_stateSessionsMutex);
        auto range = d_stateSessions.equal_range(tracker->name());
        if (range.first != range.second)
        {
          shared_ptr<StateObject const> obj = tracker->stateBase();

          if (obj)
          {
            StringBuffer buffer;
            Writer<StringBuffer> writer(buffer);

            obj->writeJson(writer);

            auto bytes = JsonSession::createBytes(buffer);

            for (auto session = range.first; session != range.second; ++session)
            {
              // If queue is too long, deal with it
              const int MaxQueueSize = 200;
              static int maxQueueSeen = 0;
              int queueSize = session->second->queue.size();
              if (queueSize/10 > maxQueueSeen/10)
              {
                maxQueueSeen = queueSize;
                log::warning("AgentStateUpdated") << tracker->name() << " max queue seen " << queueSize;
              }
              if (queueSize > MaxQueueSize)
              {
                log::error("AgentStateUpdated") << "JsonSession queue too long (" << queueSize << " > " << MaxQueueSize << "), purging";
                queue<shared_ptr<vector<uchar> const>> empty;
                swap(session->second->queue, empty);
              }

              session->second->queue.push(bytes);
            }

            libwebsocket_callback_on_writable_all_protocol(tracker->websocketProtocol);
          }
        }
      }
    );

    // Listen for Setting<T> changes and publish them via websockets
    Config::updated.connect(
      [this](SettingBase* setting)
      {
        // These should only be changed by websocket users
        assert(ThreadUtil::isDataStreamerThread());

        if (d_controlSessions.size() == 0)
          return;

        auto bytes = prepareSettingUpdateBytes(setting);

        for (JsonSession* session : d_controlSessions)
          session->queue.push(bytes);

        libwebsocket_callback_on_writable_all_protocol(d_controlProtocol);
      }
    );
  }

  log::info("DataStreamer::DataStreamer") << "Starting DataStreamer thread";
  d_thread = std::thread(&DataStreamer::run, this);
}
