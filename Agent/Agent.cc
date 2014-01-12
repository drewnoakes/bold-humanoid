#include "agent.ih"

Agent::Agent()
  : d_isRunning(false),
    d_isStopRequested(false),
    d_teamNumber(Config::getStaticValue<int>("team-number")),
    d_uniformNumber(Config::getStaticValue<int>("team-number")),
    d_cycleNumber(0)
{
  ThreadId::setThreadId(ThreadId::ThinkLoop);

  if (Config::getStaticValue<bool>("use-speech"))
    d_voice = make_shared<Voice>(Config::getStaticValue<string>("hardware.voice"));

  auto cm730DevicePath = Config::getStaticValue<string>("hardware.cm730-path");
  log::info("Agent::Agent") << "Using CM730 Device Path: " << cm730DevicePath;

  vector<shared_ptr<MotionScript>> motionScripts = MotionScript::loadAllInPath("./motionscripts");

  registerStateTypes();

  auto debugControl = make_shared<DebugControl>();
  d_debugger = make_shared<Debugger>(debugControl);

  // Register state observers
  d_fallDetector = make_shared<FallDetector>();
  d_gyroCalibrator = make_shared<GyroCalibrator>();
  d_healthAndSafety = make_shared<HealthAndSafety>(d_voice);
  d_suicidePill = make_shared<SuicidePill>(this, d_debugger);

  AgentState::getInstance().registerObserver<HardwareState>(d_fallDetector);
  AgentState::getInstance().registerObserver<HardwareState>(d_gyroCalibrator);
  AgentState::getInstance().registerObserver<HardwareState>(d_healthAndSafety);
  AgentState::getInstance().registerObserver<HardwareState>(d_suicidePill);

  auto cm730Linux = unique_ptr<CM730Linux>(new CM730Linux(cm730DevicePath));
  auto cm730 = unique_ptr<CM730>(new CM730(move(cm730Linux)));

  // Prepare the motion schedule, that coordinates which motions are carried out
  d_motionSchedule = make_shared<MotionTaskScheduler>();

  // Create motion modules
  d_headModule = make_shared<HeadModule>(d_motionSchedule);
  d_walkModule = make_shared<WalkModule>(d_motionSchedule);
  d_motionScriptModule = make_shared<MotionScriptModule>(d_motionSchedule, motionScripts);

  d_haveBody = cm730->connect();

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

  d_debugger->update();

  if (d_haveBody)
  {
    d_motionLoop = make_shared<MotionLoop>(move(cm730), debugControl);

    d_motionLoop->addModule(d_motionScriptModule);
    d_motionLoop->addModule(d_walkModule);
    d_motionLoop->addModule(d_headModule);
  }
  else
  {
    log::error("Agent::Agent") << "Failed to connect to CM730 -- continuing without motion loop";
  }
}
