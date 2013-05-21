#include "agent.ih"

Agent::Agent()
  : Configurable("agent"),
    d_isRunning(false)
{
  cout << "[Agent::Agent] Start" << endl;

  cout << "U2D dev name: " << getParam("u2dDevName") << endl;

  /*
  registerStateTypes();

  // Register state observers
  d_fallDetector = make_shared<FallDetector>();
  AgentState::getInstance().registerObserver<HardwareState>(d_fallDetector);

  d_gyroCalibrator = make_shared<GyroCalibrator>();
  AgentState::getInstance().registerObserver<HardwareState>(d_gyroCalibrator);

  d_cm730Linux = make_shared<CM730Linux>(U2D_dev.c_str());
  d_cm730 = make_shared<CM730>(d_cm730Linux);
//   d_cm730->DEBUG_PRINT = true;

  // Create motion modules
  d_walkModule = make_shared<WalkModule>(d_ini);
  d_actionModule = make_shared<ActionModule>();
  d_actionModule->loadFile(d_motionFile);
  d_headModule = make_shared<HeadModule>(d_ini);

  // Attempt to connect to the CM730
  d_haveBody = d_cm730->connect();
  
  if (!d_haveBody)
    cout << "[Agent::Agent] Unable to connect to body" << endl;

  d_ambulator = make_shared<Ambulator>(d_walkModule, d_ini),

  d_cameraModel = make_shared<CameraModel>(d_ini);

  d_spatialiser = make_shared<Spatialiser>(d_cameraModel);

  d_fieldMap = make_shared<FieldMap>(d_ini);

  d_debugger = make_shared<Debugger>();

  d_localiser = make_shared<Localiser>(d_fieldMap, d_ini);

  d_visualCortex = make_shared<VisualCortex>(d_cameraModel, d_fieldMap, d_spatialiser, d_debugger, d_headModule, d_ini);

  d_gameStateReceiver = make_shared<GameStateReceiver>(d_ini, d_debugger);

  if (useJoystick)
  {
    d_joystick = make_shared<Joystick>(1);
    d_joystickXAmpMax = d_ini.getd("Joystick", "XAmpMax", 15);
    d_joystickYAmpMax = d_ini.getd("Joystick", "YAmpMax", 15);
    d_joystickAAmpMax = d_ini.getd("Joystick", "AAmpMax", 15);
  }

  initCamera(d_ini);

  // TODO only stream if argument specified?
  d_streamer = make_shared<DataStreamer>(d_ini, d_camera, d_debugger);

  // TODO a better abstraction over control providers
  d_streamer->registerControls("camera", d_camera->getControls());
  d_streamer->registerControls("localiser", d_localiser->getControls());
  d_streamer->registerControls("ambulator", d_ambulator->getControls());
  for (auto const& pair : d_visualCortex->getControlsByFamily())
    d_streamer->registerControls(pair.first, pair.second);

  d_debugger->update(d_cm730);

  if (d_haveBody)
  {
    readStaticHardwareState();

    d_motionLoop = make_shared<MotionLoop>(d_cm730);

<<<<<<< HEAD
  d_haveBody = initMotionManager(d_ini);
  */
=======
    d_motionLoop->addModule(d_actionModule);
    d_motionLoop->addModule(d_headModule);
    d_motionLoop->addModule(d_walkModule);
  }
  else
  {
    cerr << "[Agent::Agent] Failed to connect to CM730 -- continuing without motion system" << endl;
  }
>>>>>>> feature/motion

  cout << "[Agent::Agent] Done" << endl;
}
