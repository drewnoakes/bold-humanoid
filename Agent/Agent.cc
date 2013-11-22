#include "agent.ih"

Agent::Agent()
  : d_isRunning(false),
    d_isStopRequested(false),
    d_teamNumber(0),
    d_uniformNumber(0),
    d_autoGetUpFromFallen(true),
    d_useOptionTree(true),
    d_cycleNumber(0)
{
  ThreadId::setThreadId(ThreadId::ThinkLoop);

  cout << "[Agent::Agent] Start" << endl;

  cout << "[Agent::Agent] Creating voice" << endl;

  // TODO specify these strings in configuration

  vector<string> phrases = {
    "Bold Hearts are go!",
//     "I am a protector of the realm",
//     "What do you despise? By this are you truly known.",
//     "A day may come when the courage of men fails",
//     "Duty is heavier than a mountain",
//     "Humans have a knack for choosing precisely the things that are worst for them",
//     "Ride for ruin and the world's ending!",
//     "Kill if you will, but command me nothing!",
//     "The existence of tricks does not imply the absence of magic",
//     "We eat ham and jam and Spam a lot"
  };
  srand(time(NULL));
  d_voice = make_shared<Voice>(Config::getStaticValue<string>("hardware.voice"));
  d_voice->say(phrases[rand() % phrases.size()]);

  auto cm730DevicePath = Config::getStaticValue<string>("hardware.cm730-path");
  cout << "[Agent::Agent] Using CM730 Device Path: " << cm730DevicePath << endl;

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

  // Attempt to connect to the CM730
  d_haveBody = d_cm730->connect();

  if (!d_haveBody)
    cout << "[Agent::Agent] Unable to connect to body" << endl;

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

  string sayings[] = {
    "Hello", "Bold Hearts", "Hooray", "Oh my",
    "The rain in spain falls mainly in the plain"
  };
  for (auto saying : sayings)
  {
    stringstream id;
    id << "voice.speak." << saying;
    Config::addAction(id.str(), saying, [this,saying](){ d_voice->say(saying); });
  }

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
    cerr << ccolor::fore::lightred << "[Agent::Agent] Failed to connect to CM730 -- continuing without motion system" << ccolor::reset << endl;
  }

  cout << "[Agent::Agent] Done" << endl;
}
