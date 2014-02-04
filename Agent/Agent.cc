#include "agent.ih"

Agent::Agent()
  : d_isRunning(false),
    d_isStopRequested(false),
    d_teamNumber(Config::getStaticValue<int>("team-number")),
    d_uniformNumber(Config::getStaticValue<int>("uniform-number")),
    d_cycleNumber(0)
{
  ThreadUtil::setThreadId(ThreadId::ThinkLoop);

  AgentState::initialise();

  d_voice = make_shared<Voice>();

  vector<shared_ptr<MotionScript>> motionScripts = MotionScript::loadAllInPath("./motionscripts");

  registerStateTypes();

  auto debugControl = make_shared<DebugControl>();
  d_debugger = make_shared<Debugger>(debugControl);

  // Prepare the motion schedule, that coordinates which motions are carried out
  d_motionSchedule = make_shared<MotionTaskScheduler>();

  // Create motion modules
  d_headModule = make_shared<HeadModule>(d_motionSchedule);
  d_walkModule = make_shared<WalkModule>(d_motionSchedule);
  d_motionScriptModule = make_shared<MotionScriptModule>(d_motionSchedule, motionScripts);

  // Create StateObservers
  d_fallDetector = make_shared<FallDetector>(d_voice);
  d_gyroCalibrator = make_shared<GyroCalibrator>();
  d_healthAndSafety = make_shared<HealthAndSafety>(d_voice);
  d_jamTracker = make_shared<JamDetector>(d_voice);
  d_suicidePill = make_shared<SuicidePill>(this, d_debugger);
  d_odometer = make_shared<Odometer>(d_walkModule);
  d_orientationTracker = make_shared<OrientationTracker>();

  // Register StateObservers
  AgentState::registerObserver(d_fallDetector);
  AgentState::registerObserver(d_gyroCalibrator);
  AgentState::registerObserver(d_healthAndSafety);
  AgentState::registerObserver(d_jamTracker);
  AgentState::registerObserver(d_suicidePill);
  AgentState::registerObserver(d_odometer);
  AgentState::registerObserver(d_orientationTracker);

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

  d_motionLoop = make_shared<MotionLoop>(debugControl);
  d_motionLoop->addModule(d_motionScriptModule);
  d_motionLoop->addModule(d_walkModule);
  d_motionLoop->addModule(d_headModule);
}
