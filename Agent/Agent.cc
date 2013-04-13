#include "agent.ih"

Agent::Agent(string const& U2D_dev,
             minIni const& ini,
             string const& motionFile,
             bool const& useJoystick,
             bool const& autoGetUpFromFallen,
             bool const& recordFrames,
             unsigned int const& gameControlUdpPort
  )
  : d_ini(ini),
    d_motionFile(motionFile),
    d_isRecordingFrames(recordFrames),
    d_autoGetUpFromFallen(autoGetUpFromFallen),
    d_linuxCM730(U2D_dev.c_str()),
    d_CM730(&d_linuxCM730),
    d_gameControlReceiver(gameControlUdpPort),
    d_ballSeenCnt(0),
    d_goalSeenCnt(0)
{
  cout << "[Agent::Agent] Start" << endl;

  int imageWidth = d_ini.geti("Camera", "ImageWidth", 320);
  int imageHeight = d_ini.geti("Camera", "ImageHeight", 240);
  double focalLength = d_ini.getd("Camera", "FocalLength", 0.025);
  double rangeVerticalDegs = d_ini.getd("Camera", "RangeVerticalDegrees", 46.0);
  double rangeHorizontalDegs = d_ini.getd("Camera", "RangeHorizontalDegrees", 58.0);
  // TODO have seen both 58.0 and 60.0 as default horizontal range values

  d_ambulator = make_shared<Ambulator>(d_ini),

  d_cameraModel = make_shared<CameraModel>(imageWidth, imageHeight, focalLength, rangeVerticalDegs, rangeHorizontalDegs);

  d_spatialiser = make_shared<Spatialiser>(d_cameraModel);

  d_visualCortex = make_shared<VisualCortex>(d_cameraModel);
  d_visualCortex->initialise(ini);

  if (useJoystick)
  {
    d_joystick = make_shared<Joystick>(1);
    d_joystickXAmpMax = d_ini.getd("Joystick", "XAmpMax", 15);
    d_joystickYAmpMax = d_ini.getd("Joystick", "YAmpMax", 15);
    d_joystickAAmpMax = d_ini.getd("Joystick", "AAmpMax", 15);
  }

  d_circleBallX = d_ini.getd("Circle Ball", "WalkX", -1);
  d_circleBallY = d_ini.getd("Circle Ball", "WalkY", 50);
  d_circleBallTurn = d_ini.getd("Circle Ball", "WalkTurn", 15);

  d_camera = make_shared<Camera>("/dev/video0");

  initCamera();

  // TODO only stream if argument specified?
  // TODO port from config, not constructor
  d_streamer = make_shared<DataStreamer>(8080, d_ini, d_camera);

  d_streamer->registerControls("camera", d_camera->getControls());
  for (auto const& pair : d_visualCortex->getControlsByFamily())
    d_streamer->registerControls(pair.first, pair.second);

  Debugger::getInstance().update(d_CM730);

  // Sit action
  OptionPtr sit = make_shared<ActionOption>("sitdownaction","sit down");
  d_optionTree.addOption(sit);

  // Build main FSM
  auto fsm = make_shared<FSMOption>("lookforball");
  d_optionTree.addOption(fsm, true /*top*/);
  auto lookAround = make_shared<LookAround>("lookaround");
  d_optionTree.addOption(lookAround);
  auto lookAtBall = make_shared<LookAtBall>("lookatball", d_cameraModel);
  d_optionTree.addOption(lookAtBall);

  // Start state: look around
  auto lookAroundState = fsm->newState("lookaround", {lookAround}, false/*endState*/, true/*startState*/);

  // Transition: look at ball when visible
  auto lookAround2lookAtBall = lookAroundState->newTransition();
  lookAround2lookAtBall->condition = []() {
    return AgentState::getInstance().cameraFrame()->isBallVisible();
  };

  // Next state: look at ball
  auto lookAtBallState = fsm->newState("lookatball", {lookAtBall});
  lookAround2lookAtBall->nextState = lookAtBallState;

  // Transition: look for ball if no longer seen
  auto lookAtBall2lookAround = lookAtBallState->newTransition();
  lookAtBall2lookAround->condition = []() {
    return !AgentState::getInstance().cameraFrame()->isBallVisible();
  };
  lookAtBall2lookAround->nextState = lookAroundState;

  d_haveBody = initMotionManager();

  d_state = State::S_INIT;

  cout << "[Agent::Agent] Done" << endl;
}

