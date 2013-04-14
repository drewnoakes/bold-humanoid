#include "adhocoptiontreebuilder.ih"

unique_ptr<OptionTree> AdHocOptionTreeBuilder::buildTree(minIni const& ini)
{
  unique_ptr<OptionTree> tree(new OptionTree());

  auto cameraModel = make_shared<CameraModel>(ini);
  // Sit down action
  OptionPtr sit = make_shared<ActionOption>("sitdownaction","sit down");
  tree->addOption(sit, true);
  // Stand up action
  OptionPtr standup = make_shared<ActionOption>("standupaction","stand up");
  tree->addOption(standup);

  // Stand action
  OptionPtr stand = make_shared<Stand>("stand", make_shared<Ambulator>(ini));
  tree->addOption(stand);

  // Build main FSM
  auto fsm = make_shared<FSMOption>("lookforball");
  tree->addOption(fsm);
  auto lookAround = make_shared<LookAround>("lookaround");
  tree->addOption(lookAround);
  auto lookAtBall = make_shared<LookAtBall>("lookatball", cameraModel);
  tree->addOption(lookAtBall);

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

  return tree;
}
