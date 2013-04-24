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

  // Left kick
  OptionPtr leftKick = make_shared<ActionOption>("leftkickaction","lk");
  tree->addOption(leftKick);
  
  // Left kick
  OptionPtr rightKick = make_shared<ActionOption>("rightkickaction","rk");
  tree->addOption(rightKick);

  // Look around
  OptionPtr lookAround = make_shared<LookAround>("lookaround");
  tree->addOption(lookAround);

  // Look at ball
  OptionPtr lookAtBall = make_shared<LookAtBall>("lookatball", cameraModel);
  tree->addOption(lookAtBall);
  
  // Look at goal
  OptionPtr lookAtGoal = make_shared<LookAtGoal>("lookatgoal", cameraModel);
  tree->addOption(lookAtGoal);
  
  // FSM
  auto winFsm = make_shared<FSMOption>("win");
  tree->addOption(winFsm, true);
  auto attackFsm = make_shared<FSMOption>("attack");
  tree->addOption(attackFsm);


  //
  // Build main FSM
  //
  // State: paused
  auto pauseState = winFsm->newState("pause", {stand}, false, true);

  // State: attack
  auto attackState = winFsm->newState("attack", {attackFsm});

  auto startButtonCondition = []() {
    static int lastSwitch = 0;
    lastSwitch++;
    if (lastSwitch < 20)
      return false;

    auto hw = AgentState::get<HardwareState>();
    if (!hw)
      return false;
    auto cm730 = hw->getCM730State();
    if (!cm730)
      return false;

    if (cm730->isStartButtonPressed)
    {
      lastSwitch = 0;
      return true;;
    }
    
    return false;
  };

  auto penaltyTransition = []() {
    auto gameState = AgentState::getInstance().get<GameState>();
    int ourTeam = 0; //TODO: get from where?
    int myUnum = 0; //TODO: get from where?
    auto myGameStateInfo = gameState->playerInfo(ourTeam, myUnum);
    return myGameStateInfo.hasPenalty();
  };

  auto noPenaltyTransition = [&penaltyTransition]() {
    return !penaltyTransition();
  };

  auto readyTransition = []() {
    auto gameState = AgentState::getInstance().get<GameState>();
    return gameState->getPlayMode() == PlayMode::READY;
  };

  auto setTransition = []() {
    auto gameState = AgentState::getInstance().get<GameState>();
    return gameState->getPlayMode() == PlayMode::SET;
  };

  auto playingTransition = []() {
    auto gameState = AgentState::getInstance().get<GameState>();
    return gameState->getPlayMode() == PlayMode::PLAYING;
  };

  auto finishedTransition = []() {
    auto gameState = AgentState::getInstance().get<GameState>();
    return gameState->getPlayMode() == PlayMode::FINISHED;
  };

  auto pause2attack = pauseState->newTransition();
  pause2attack->condition = startButtonCondition;
  pause2attack->childState = attackState;

  auto attack2pause = attackState->newTransition();
  attack2pause->condition = startButtonCondition;
  attack2pause->childState = pauseState;

  //
  // Build attack FSM
  //
  // Start state: stand up
  auto standUpState = attackFsm->newState("standup", {standup}, false/*endState*/, true/*startState*/);

  // State: stand and look look around
  auto lookAroundState = attackFsm->newState("lookaround", {stand, lookAround});

  // State: stand and look at ball
  auto lookAtBallState = attackFsm->newState("lookatball", {stand, lookAtBall});

  // State: approach and look at ball
  auto approachBallState = attackFsm->newState("approachball", {approachBall, lookAtBall});

  auto ballLostCondition = []() {
    static int lastSeen = 0;
    lastSeen++;
    if (AgentState::get<CameraFrameState>()->isBallVisible())
      lastSeen = 0;
    return lastSeen > 10;
  };

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
    return AgentState::get<CameraFrameState>()->isBallVisible();
  };
  lookAround2lookAtBall->childState = lookAtBallState;

  // Transition: look for ball if no longer seen
  auto lookAtBall2lookAround = lookAtBallState->newTransition();
  lookAtBall2lookAround->condition = ballLostCondition;
  lookAtBall2lookAround->childState = lookAroundState;

  // Transition: approach ball
  auto lookAtBall2approachBall = lookAtBallState->newTransition();
  lookAtBall2approachBall->condition = []() {
    static int nSeen = 0;
    if (AgentState::get<CameraFrameState>()->isBallVisible())
      nSeen++;
    else if (nSeen > 0)
      nSeen--;
    return nSeen > 10;
  };
  lookAtBall2approachBall->childState = approachBallState;

  // Transition: look for ball again if no longer seen
  auto approachBall2lookAround = approachBallState->newTransition();
  approachBall2lookAround->condition = ballLostCondition;
  approachBall2lookAround->childState = lookAroundState;

  return tree;
}
