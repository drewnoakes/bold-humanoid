#include "agent.ih"

Agent::Agent(string const& U2D_dev,
             minIni const& ini,
             string const& motionFile,
             bool const& useJoystick,
             bool const& autoGetUpFromFallen,
             bool const& useOptionTree,
             bool const& recordFrames,
             unsigned int const& gameControlUdpPort
  )
  : d_motionFile(motionFile),
    d_isRecordingFrames(recordFrames),
    d_autoGetUpFromFallen(autoGetUpFromFallen),
    d_useOptionTree(useOptionTree),
    d_gameControlReceiver(gameControlUdpPort),
    d_ballSeenCnt(0),
    d_goalSeenCnt(0)
{
  cout << "[Agent::Agent] Start" << endl;

  registerStateTypes();

  d_linuxCM730 = make_shared<LinuxCM730>(U2D_dev.c_str());
  d_CM730 = make_shared<CM730>(d_linuxCM730.get());

  d_ambulator = make_shared<Ambulator>(ini),

  d_cameraModel = make_shared<CameraModel>(ini);

  d_spatialiser = make_shared<Spatialiser>(d_cameraModel);

  d_fieldMap = make_shared<FieldMap>(ini);

  d_debugger = make_shared<Debugger>();

  d_visualCortex = make_shared<VisualCortex>(d_cameraModel, d_fieldMap, d_debugger, ini);

  AdHocOptionTreeBuilder optionTreeBuilder;
  d_optionTree = optionTreeBuilder.buildTree(ini);

  if (useJoystick)
  {
    d_joystick = make_shared<Joystick>(1);
    d_joystickXAmpMax = ini.getd("Joystick", "XAmpMax", 15);
    d_joystickYAmpMax = ini.getd("Joystick", "YAmpMax", 15);
    d_joystickAAmpMax = ini.getd("Joystick", "AAmpMax", 15);
  }

  d_circleBallX = ini.getd("Circle Ball", "WalkX", -1);
  d_circleBallY = ini.getd("Circle Ball", "WalkY", 50);
  d_circleBallTurn = ini.getd("Circle Ball", "WalkTurn", 15);

  initCamera(ini);

  // TODO only stream if argument specified?
  d_streamer = make_shared<DataStreamer>(ini, d_camera, d_debugger);

  d_streamer->registerControls("camera", d_camera->getControls());
  for (auto const& pair : d_visualCortex->getControlsByFamily())
    d_streamer->registerControls(pair.first, pair.second);

  d_debugger->update(d_CM730);


  d_haveBody = initMotionManager(ini);

  d_state = State::S_INIT;

  cout << "[Agent::Agent] Done" << endl;
}

