#include "adhocoptiontreebuilder.ih"

unique_ptr<OptionTree> AdHocOptionTreeBuilder::buildTree(minIni const& ini)
{
  unique_ptr<OptionTree> tree(new OptionTree());

  auto cameraModel = make_shared<CameraModel>(ini);
  auto ambulator = make_shared<Ambulator>(ini);

  // Sit down action
  OptionPtr sit = make_shared<ActionOption>("sitdownaction","sit down");
  tree->addOption(sit);
  // Stand up action
  OptionPtr standup = make_shared<ActionOption>("standupaction","stand up");
  tree->addOption(standup);

  // Stand
  OptionPtr stand = make_shared<Stand>("stand", ambulator);
  tree->addOption(stand);

  // Approach ball
  OptionPtr approachBall = make_shared<ApproachBall>("approachball", ambulator);
  tree->addOption(approachBall);


  // FSM
  auto fsm = make_shared<FSMOption>("win");
  tree->addOption(fsm, true);
  auto lookAround = make_shared<LookAround>("lookaround");
  tree->addOption(lookAround);
  auto lookAtBall = make_shared<LookAtBall>("lookatball", cameraModel);
  tree->addOption(lookAtBall);

  // Build main FSM

  // Start state: stand up
  auto standUpState = fsm->newState("standup", {standup}, false/*endState*/, true/*startState*/);

  // State: stand and look look around
  auto lookAroundState = fsm->newState("lookaround", {stand, lookAround});

  // State: stand and look at ball
  auto lookAtBallState = fsm->newState("lookatball", {stand, lookAtBall});

  // State: approach and look at ball
  auto approachBallState = fsm->newState("approachball", {approachBall, lookAtBall});

  // Transition: into actual loop after stood up
  auto standUp2lookAround = standUpState->newTransition();
  standUp2lookAround->condition = [standUp2lookAround]() {
    auto& os = standUp2lookAround->parentState->options;
    return std::all_of(os.begin(), os.end(), [](OptionPtr o) { return o->hasTerminated(); });
  };
  standUp2lookAround->childState = lookAroundState;

  // Transition: look at ball when visible
  auto lookAround2lookAtBall = lookAroundState->newTransition();
  lookAround2lookAtBall->condition = []() {
    return AgentState::getInstance().cameraFrame()->isBallVisible();
  };
  lookAround2lookAtBall->childState = lookAtBallState;

  // Transition: look for ball if no longer seen
  auto lookAtBall2lookAround = lookAtBallState->newTransition();
  lookAtBall2lookAround->condition = []() {
    static int lastSeen = 0;
    lastSeen++;
    if (AgentState::getInstance().cameraFrame()->isBallVisible())
      lastSeen = 0;
    return lastSeen > 10;
  };
  lookAtBall2lookAround->childState = lookAroundState;

  // Transition: approach ball
  auto lookAtBall2approachBall = lookAtBallState->newTransition();
  lookAtBall2approachBall->condition = []() {
    static int nSeen = 0;
    if (AgentState::getInstance().cameraFrame()->isBallVisible())
      nSeen++;
    else if (nSeen > 0)
      nSeen--;
    return nSeen > 10;
  };
  lookAtBall2approachBall->childState = approachBallState;

  return tree;
}
