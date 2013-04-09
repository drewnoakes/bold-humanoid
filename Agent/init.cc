#include "agent.ih"

bool Agent::init()
{
  cout << "[Agent::init] Start" << endl;

  initCamera();

  // Check if camera is opened successfully
  /*
  if (!d_camera.isOpened())
  {
    cout << "[Agent::init] Failed to open camera!" << endl;
    return false;
  }
  */

  // TODO only stream if argument specified?
  // TODO port from config, not constructor
  d_streamer = make_shared<DataStreamer>(8080);
  d_streamer->initialise(d_ini);
  d_streamer->setCamera(d_camera);

  d_streamer->registerControls("camera", d_camera->getControls());
  for (auto const& pair : VisualCortex::getInstance().getControlsByFamily())
    d_streamer->registerControls(pair.first, pair.second);

  Debugger::getInstance().update(d_CM730);

  AgentModel::getInstance().initialise(/*d_ini*/);

  // Sit action
  OptionPtr sit = make_shared<ActionOption>("sitdownaction","sit down");
  d_optionTree.addOption(sit);

  // Build main FSM
  auto fsm = make_shared<FSMOption>("lookforball");
  d_optionTree.addOption(fsm, true /*top*/);
  auto lookAround = make_shared<LookAround>("lookaround");
  d_optionTree.addOption(lookAround);
  auto lookAtBall = make_shared<LookAtBall>("lookatball");
  d_optionTree.addOption(lookAtBall);

  // Start state: look around
  auto lookAroundState = fsm->newState("lookaround", lookAround, false/*endState*/, true/*startState*/);

  // Transition: look at ball when visible
  auto lookAround2lookAtBall = lookAroundState->newTransition();
  lookAround2lookAtBall->condition = []() {
    auto& vision = VisualCortex::getInstance();
    return vision.isBallVisible();
  };

  // Next state: look at ball
  auto lookAtBallState = fsm->newState("lookatball", lookAtBall);
  lookAround2lookAtBall->nextState = lookAtBallState;

  // Transition: look for ball if no longer seen
  auto lookAtBall2lookAround = lookAtBallState->newTransition();
  lookAtBall2lookAround->condition = []() {
    auto& vision = VisualCortex::getInstance();
    return !vision.isBallVisible();
  };
  lookAtBall2lookAround->nextState = lookAroundState;

  d_haveBody = initBody();

  d_state = S_INIT;

  cout << "[Agent::init] Done" << endl;

  return true;
}
