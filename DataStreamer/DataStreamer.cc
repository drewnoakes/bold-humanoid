#include "datastreamer.ih"

DataStreamer::DataStreamer(minIni const& ini, shared_ptr<Camera> camera)
: d_image(),
  d_imageType(ImageType::RGB),
  d_streamFramePeriod(10),
  d_shouldDrawBlobs(true),
  d_shouldDrawLineDots(false),
  d_shouldDrawExpectedLines(true),
  d_shouldDrawObservedLines(true),
  d_camera(camera),
  d_cameraSessions(),
  d_controlsByIdByFamily(),
  d_context(0)
{
  cout << "[DataStreamer::DataStreamer] Starting" << endl;

  d_port = ini.geti("Debugger", "TcpPort", 8080);

  d_streamFramePeriod = ini.geti("Debugger", "BroadcastFramePeriod", 5);

  // We have three special protocols: HTTP-only, Camera and Timing.
  // These are followed by N other protocols, one per type of state in the system

  auto stateTypes = AgentState::getInstance().allStateTypes();

  unsigned protocolCount = 4 + stateTypes.size();
  d_protocols = new libwebsocket_protocols[protocolCount];

                   // name, callback, per-session-data-size, rx-buffer-size, owning-server, protocol-index
  d_protocols[0] = { "http-only", DataStreamer::_callback_http, 0, 0, NULL, 0 };
  d_protocols[1] = { "camera-protocol", DataStreamer::_callback_camera, sizeof(CameraSession), 0, NULL, 0 };
  d_protocols[2] = { "timing-protocol", DataStreamer::_callback_timing, 0, 0, NULL, 0 };

  // TODO review the storage/recovery of protocols as given here

  auto getStateName = [](StateType type)->char*
  {
    switch (type)
    {
      case StateType::AgentFrame:
        return (char*)"AgentFrame";
      case StateType::Alarm:
        return (char*)"Alarm";
      case StateType::Body:
        return (char*)"Body";
      case StateType::CameraFrame:
        return (char*)"CameraFrame";
      case StateType::Game:
        return (char*)"Game";
      case StateType::Hardware:
        return (char*)"Hardware";
      default:
        throw new runtime_error("Unsupported enum class value.");
    }
  };

  // One protocol per state
  map<StateType, libwebsocket_protocols*> protocolByStateType;
  unsigned protocolIndex = 3;
  for (StateType const& stateType : stateTypes)
  {
    d_protocols[protocolIndex] = { getStateName(stateType), DataStreamer::_callback_state, 0, 0, NULL, 0 };
    protocolByStateType[stateType] = &d_protocols[protocolIndex];
    protocolIndex++;
  }

  // Mark the end of the protocols
  d_protocols[protocolIndex] = { NULL, NULL, 0, 0, NULL, 0 };

  d_cameraProtocol = &d_protocols[1];
  d_timingProtocol = &d_protocols[2];

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

  AgentState::getInstance().updated.connect(
    [protocolByStateType](StateType type, shared_ptr<StateObject> obj) {
      auto i = protocolByStateType.find(type);
      if (i != protocolByStateType.end())
      {
        libwebsocket_protocols* p = i->second;
        libwebsocket_callback_on_writable_all_protocol(p);
      }
    }
  );

  // TODO split into vision and head controls
  registerControls("debug", getDebugControls());
}
