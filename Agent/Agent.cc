#include "agent.ih"

Agent::Agent()
  : d_isRunning(false),
    d_isStopRequested(false),
    d_teamNumber(Config::getStaticValue<int>("team-number")),
    d_uniformNumber(Config::getStaticValue<int>("uniform-number")),
    d_cycleNumber(0)
{
  ThreadUtil::setThreadId(ThreadId::ThinkLoop);

  State::initialise();

  d_voice = make_shared<Voice>();
  d_behaviourControl = make_shared<BehaviourControl>(*this);

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
  d_vocaliser = make_shared<Vocaliser>(d_voice);
  d_fallDetector = make_shared<FallDetector>(d_voice);
  d_gyroCalibrator = make_shared<GyroCalibrator>();
  d_healthAndSafety = make_shared<HealthAndSafety>(d_voice);
  d_jamTracker = make_shared<JamDetector>(d_voice);
  d_suicidePill = make_shared<SuicidePill>(this, d_debugger);
  d_odometer = make_shared<Odometer>(d_walkModule);
  d_orientationTracker = make_shared<OrientationTracker>();
  d_teamCommunicator = make_shared<OpenTeamCommunicator>(d_behaviourControl);

  // Register StateObservers
  State::registerObserver(d_vocaliser);
  State::registerObserver(d_fallDetector);
  State::registerObserver(d_gyroCalibrator);
  State::registerObserver(d_healthAndSafety);
  State::registerObserver(d_jamTracker);
  State::registerObserver(d_suicidePill);
  State::registerObserver(d_odometer);
  State::registerObserver(d_teamCommunicator);
  State::registerObserver(d_orientationTracker);

  d_ambulator = make_shared<Ambulator>(d_walkModule),

  d_cameraModel = allocate_aligned_shared<CameraModel>();

  d_spatialiser = make_shared<Spatialiser>(d_cameraModel);

  d_fieldMap = make_shared<FieldMap>();

  d_localiser = allocate_aligned_shared<Localiser>(d_fieldMap);

  d_roleDecider = make_shared<RoleDecider>(d_behaviourControl, d_debugger);

  // Create camera
  d_camera = make_shared<Camera>(Config::getStaticValue<string>("hardware.video-path"));
  d_camera->open();

  // Configure camera
  unsigned width = d_cameraModel->imageWidth();
  unsigned height = d_cameraModel->imageHeight();
  if (!d_camera->getPixelFormat().requestSize(width, height))
    log::error() << "[Agent::initCamera] Requesting camera size " << width << "x" << height << " failed";

  auto pixelFormat = d_camera->getPixelFormat();
  log::info("Agent::initCamera") << "Camera resolution: " << pixelFormat.width << "x" << pixelFormat.height;

  // Start capturing camera images
  d_camera->startCapture();

  d_streamer = make_shared<DataStreamer>(d_camera);

  d_visualCortex = make_shared<VisualCortex>(d_camera, d_cameraModel, d_streamer, d_fieldMap, d_spatialiser, d_headModule);

  d_gameStateReceiver = make_shared<GameStateReceiver>(d_debugger, d_voice);

  if (Config::getStaticValue<bool>("hardware.joystick.enabled"))
  {
    d_joystick = make_shared<Joystick>(1);
    d_joystickXAmpMax = Config::getSetting<double>("hardware.joystick.x-amp-max");
    d_joystickYAmpMax = Config::getSetting<double>("hardware.joystick.y-amp-max");
    d_joystickAAmpMax = Config::getSetting<double>("hardware.joystick.a-amp-max");
  }

  d_debugger->update();

  d_motionLoop = make_shared<MotionLoop>(debugControl);
  d_motionLoop->addModule(d_motionScriptModule);
  d_motionLoop->addModule(d_walkModule);
  d_motionLoop->addModule(d_headModule);

  d_motionLoop->onReadFailure.connect([this](uint count) -> void {
    // If we were unable to read once during the last second, then we've lost
    // the CM730. This is probably because someone pressed the hardware reset
    // button on the back. In this case, stop the agent. The upstart service
    // should restart it immediately during competitions.
    // TODO consider reconnecting to the CM730, as we already have a means of exiting the robot by holding down two buttons
    // TODO place read failure timeout duration in config
    if (count > 1000 / 8) // 1 second
    {
      log::error("Agent") << "MotionLoop has been unable to read " << count << " times, so shutting down";
      stop();
    }
  });
}
