#include "agent.ih"

Agent::Agent(string const& U2D_dev,
             minIni const& ini,
             string const& motionFile,
             unsigned teamNumber,
             unsigned uniformNumber,
             bool useJoystick,
             bool autoGetUpFromFallen,
             bool useOptionTree,
             bool recordFrames,
             bool ignoreGameController
  )
  : d_isRunning(false),
    d_motionFile(motionFile),
    d_teamNumber(teamNumber),
    d_uniformNumber(uniformNumber),
    d_isRecordingFrames(recordFrames),
    d_autoGetUpFromFallen(autoGetUpFromFallen),
    d_useOptionTree(useOptionTree),
    d_ignoreGameController(ignoreGameController),
    d_ballSeenCnt(0),
    d_goalSeenCnt(0)
{
  cout << "[Agent::Agent] Start" << endl;

  registerStateTypes();

  d_linuxCM730 = make_shared<LinuxCM730>(U2D_dev.c_str());
  d_CM730 = make_shared<CM730>(d_linuxCM730.get());
  d_CM730->MakeBulkReadPacket();

  d_ambulator = make_shared<Ambulator>(ini),

  d_cameraModel = make_shared<CameraModel>(ini);

  d_spatialiser = make_shared<Spatialiser>(d_cameraModel);

  d_fieldMap = make_shared<FieldMap>(ini);

  d_debugger = make_shared<Debugger>();

  d_localiser = make_shared<Localiser>(d_fieldMap, ini);

  d_visualCortex = make_shared<VisualCortex>(d_cameraModel, d_fieldMap, d_spatialiser, d_debugger, ini);

  d_gameStateReceiver = make_shared<GameStateReceiver>(ini, d_debugger);

  AdHocOptionTreeBuilder optionTreeBuilder;
  d_optionTree = optionTreeBuilder.buildTree(ini,
                                             d_teamNumber,
                                             d_uniformNumber,
                                             d_ignoreGameController,
                                             d_debugger,
                                             d_cameraModel,
                                             d_ambulator);

  if (useJoystick)
  {
    d_joystick = make_shared<Joystick>(1);
    d_joystickXAmpMax = ini.getd("Joystick", "XAmpMax", 15);
    d_joystickYAmpMax = ini.getd("Joystick", "YAmpMax", 15);
    d_joystickAAmpMax = ini.getd("Joystick", "AAmpMax", 15);
  }

  initCamera(ini);

  // TODO only stream if argument specified?
  d_streamer = make_shared<DataStreamer>(ini, d_camera, d_debugger);

  // TODO a better abstraction over control providers
  d_streamer->registerControls("camera", d_camera->getControls());
  d_streamer->registerControls("localiser", d_localiser->getControls());
  for (auto const& pair : d_visualCortex->getControlsByFamily())
    d_streamer->registerControls(pair.first, pair.second);

  d_debugger->update(d_CM730);

  d_haveBody = initMotionManager(ini);

  d_state = State::S_INIT;

  cout << "[Agent::Agent] Done" << endl;
}
