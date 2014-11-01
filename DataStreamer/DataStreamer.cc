#include "datastreamer.ih"

// TODO support a data protocol that supports queries and updates via JSON, all over a single websocket protocol

DataStreamer::DataStreamer(shared_ptr<Camera> camera)
  : hasClientChanged(),
    d_port(Config::getStaticValue<int>("round-table.tcp-port")),
    d_camera(camera),
    d_image(),
    d_context(0),
    d_cameraSessions(),
    d_isStopRequested(false),
    d_thread()
{
  // Set libwebsockets logging level, and redirect logging through our log framework
  lws_set_log_level(LLL_ERR | LLL_WARN, [](int level, char const* line)
  {
    // Trim the newline character
    string l(line);
    l = l.substr(0, l.length() - 1);

    if (level == LLL_ERR)
      log::error("libwebsockets") << l;
    else if (level == LLL_WARN)
      log::warning("libwebsockets") << l;
    else
      log::info("libwebsockets") << l;
  });

  // We have three special protocols: HTTP-only, Camera and Control.
  // These are followed by N other protocols, one per type of state in the system

  unsigned protocolCount = 3 + State::stateTypeCount() + 1;

  d_protocols = new libwebsocket_protocols[protocolCount];

                   // name, callback, per-session-data-size, rx-buffer-size, no-buffer-all-partial-tx
  d_protocols[0] = { "http-only",        DataStreamer::_callback_http,    0,                     0, 0 };
  d_protocols[1] = { "camera-protocol",  DataStreamer::_callback_camera,  sizeof(CameraSession), 0, 0 };
  d_protocols[2] = { "control-protocol", DataStreamer::_callback_control, sizeof(JsonSession),   0, 0 };

  d_controlProtocol = &d_protocols[2];

  // One protocol per state
  unsigned protocolIndex = 3;
  for (shared_ptr<StateTracker> stateTracker : State::getTrackers())
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
    ASSERT(libwebsocket_context_user(d_context) == this);

    // Listen for StateObject changes and publish them via websockets
    State::updated.connect(
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
            for (auto it = range.first; it != range.second; ++it)
            {
              JsonSession* session = it->second;

              WebSocketBuffer buffer;
              Writer<WebSocketBuffer> writer(buffer);
              obj->writeJson(writer);
              session->enqueue(move(buffer), /*suppressLwsNotify*/ true);
            }

            libwebsocket_callback_on_writable_all_protocol(tracker->websocketProtocol);
          }
        }
      }
    );

    // Listen for Setting<T> changes and publish them via websockets
    Config::updated.connect(
      [this](SettingBase const* setting)
      {
        // These should only be changed by websocket users
        //ASSERT(ThreadUtil::isDataStreamerThread());
        // TODO: quick workaround
        if (!ThreadUtil::isDataStreamerThread(false))
          return;

        lock_guard<mutex> guard(d_controlSessionsMutex);
        for (JsonSession* session : d_controlSessions)
        {
          WebSocketBuffer buffer;
          Writer<WebSocketBuffer> writer(buffer);
          writeSettingUpdateJson(setting, writer);
          session->enqueue(move(buffer), /*suppressLwsNotify*/ true);
        }

        libwebsocket_callback_on_writable_all_protocol(d_controlProtocol);
      }
    );
  }

  log::info("DataStreamer::DataStreamer") << "Starting DataStreamer thread";
  d_thread = std::thread(&DataStreamer::run, this);
}
