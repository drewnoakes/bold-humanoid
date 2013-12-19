#include "agent.ih"

Agent::Agent(bool useSpeech)
  : d_isRunning(false),
    d_isStopRequested(false),
    d_teamNumber(0),
    d_uniformNumber(0),
    d_cycleNumber(0)
{
  ThreadId::setThreadId(ThreadId::ThinkLoop);

  if (useSpeech)
    d_voice = make_shared<Voice>(Config::getStaticValue<string>("hardware.voice"));

  auto cm730DevicePath = Config::getStaticValue<string>("hardware.cm730-path");
  log::info("Agent::Agent") << "Using CM730 Device Path: " << cm730DevicePath;

  vector<shared_ptr<MotionScript>> motionScripts = MotionScript::loadAllInPath("./motionscripts");

  registerStateTypes();

  // Register state observers
  d_fallDetector = make_shared<FallDetector>();
  AgentState::getInstance().registerObserver<HardwareState>(d_fallDetector);

  d_gyroCalibrator = make_shared<GyroCalibrator>();
  AgentState::getInstance().registerObserver<HardwareState>(d_gyroCalibrator);

  d_cm730Linux = make_shared<CM730Linux>(cm730DevicePath);
  d_cm730 = make_shared<CM730>(d_cm730Linux);
//   d_cm730->DEBUG_PRINT = true;

  // Prepare the motion schedule, that coordinates which motions are carried out
  d_motionSchedule = make_shared<MotionTaskScheduler>();

  // Create motion modules
  d_headModule = make_shared<HeadModule>(d_motionSchedule);
  d_walkModule = make_shared<WalkModule>(d_motionSchedule);
  d_motionScriptModule = make_shared<MotionScriptModule>(d_motionSchedule, motionScripts);

  d_haveBody = d_cm730->connect();

  if (!d_haveBody)
  {
    // No body exists, so provide a 'zero' position for all joints to
    // allow better debugging of code on non-robot machines.
    AgentState::getInstance().set<BodyState>(BodyState::zero());
  }

  d_ambulator = make_shared<Ambulator>(d_walkModule),

  d_cameraModel = make_shared<CameraModel>();

  d_spatialiser = make_shared<Spatialiser>(d_cameraModel);

  d_fieldMap = make_shared<FieldMap>();

  d_debugger = make_shared<Debugger>();

  d_localiser = make_shared<Localiser>(d_fieldMap);

  initCamera();

  d_visualCortex = make_shared<VisualCortex>(d_camera, d_cameraModel, d_fieldMap, d_spatialiser, d_headModule);

  d_gameStateReceiver = make_shared<GameStateReceiver>(d_debugger, this);

  if (Config::getStaticValue<bool>("hardware.joystick.enabled"))
  {
    d_joystick = make_shared<Joystick>(1);
    d_joystickXAmpMax = Config::getSetting<double>("hardware.joystick.x-amp-max");
    d_joystickYAmpMax = Config::getSetting<double>("hardware.joystick.y-amp-max");
    d_joystickAAmpMax = Config::getSetting<double>("hardware.joystick.a-amp-max");
  }

  // TODO only stream if argument specified?
  d_streamer = make_shared<DataStreamer>(d_camera);

  d_debugger->update(d_cm730);

  if (d_haveBody)
  {
    readStaticHardwareState();

    d_motionLoop = make_shared<MotionLoop>(d_cm730);

    d_motionLoop->addModule(d_motionScriptModule);
    d_motionLoop->addModule(d_walkModule);
    d_motionLoop->addModule(d_headModule);
  }
  else
  {
    log::error("Agent::Agent") << "Failed to connect to CM730 -- continuing without motion loop";
  }
}
