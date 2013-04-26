#include "adhocoptiontreebuilder.ih"

unique_ptr<OptionTree> AdHocOptionTreeBuilder::buildTree(minIni const& ini,
                                                         unsigned teamNumber,
                                                         unsigned uniformNumber,
                                                         bool ignoreGameController,
                                                         shared_ptr<Debugger> debugger)
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

  // Circle around ball
  OptionPtr circleBall = make_shared<CircleBall>("circleball", ambulator);
  tree->addOption(circleBall);

  // Left kick
  OptionPtr leftKick = make_shared<ActionOption>("leftkickaction","lk");
  tree->addOption(leftKick);

  // Left kick
  OptionPtr rightKick = make_shared<ActionOption>("rightkickaction","rk");
  tree->addOption(rightKick);

  // Look around
  OptionPtr lookAround = make_shared<LookAround>("lookaround", ini);
  tree->addOption(lookAround);

  // Look at ball
  OptionPtr lookAtBall = make_shared<LookAtBall>("lookatball", cameraModel);
  tree->addOption(lookAtBall);

  // Look at feet
  OptionPtr lookAtFeet = make_shared<LookAtFeet>("lookatfeet", ini);
  tree->addOption(lookAtFeet);

  // Look at goal
  OptionPtr lookAtGoal = make_shared<LookAtGoal>("lookatgoal", cameraModel);
  tree->addOption(lookAtGoal);

  // FSM
  auto winFsm = make_shared<FSMOption>("win");
  tree->addOption(winFsm, true);

  auto playingFsm = make_shared<FSMOption>("playing");
  tree->addOption(playingFsm);

  //
  // ========== WIN ==========
  //

  // ---------- STATES ----------
  // State: paused
  auto pauseState = winFsm->newState("pause", {stand}, false/*end state*/, true/*start state*/);

  // State: ready
  auto readyState = winFsm->newState("ready", {stand});

  // State: set
  auto setState = winFsm->newState("set", {stand});

  // State: beforeTheirKickoff
  auto beforeTheirKickoff = winFsm->newState("beforetheirkickoff", {stand});

  // State: playing
  auto playingState = winFsm->newState("playing", {playingFsm});

  // State: penalized
  auto penalizedState = winFsm->newState("penalized", {stand});


  // ---------- TRANSITIONS ----------

  // Start button condition
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
      return true;
    }

    return false;
  };

  // Penalty condition
  auto penaltyCondition = [=]() {
    auto gameState = AgentState::get<GameState>();
    if (!gameState)
      return false;

    auto myGameStateInfo = gameState->teamInfo(teamNumber).getPlayer(uniformNumber);
    return myGameStateInfo.hasPenalty();
  };

  // No penalty condition
  // TODO: should be simply !penaltyCondition, but didn't work on first try
  auto noPenaltyCondition = [=]() {
    auto gameState = AgentState::get<GameState>();
    if (!gameState)
      return true;

    auto myGameStateInfo = gameState->teamInfo(teamNumber).getPlayer(uniformNumber);
    return !myGameStateInfo.hasPenalty();
  };

  // READY playmode condition
  auto readyCondition = [=]() {
    auto gameState = AgentState::get<GameState>();
    debugger->showPaused();
    return gameState->getPlayMode() == PlayMode::READY;
  };

  // SET playmode condition
  auto setCondition = []() {
    auto gameState = AgentState::get<GameState>();
    if (!gameState) // No gamestate yes, most likely not SET
      return false;
    return gameState->getPlayMode() == PlayMode::SET;
  };

  // PLAYING playmode condition
  auto playingCondition = []() {
    auto gameState = AgentState::get<GameState>();
    return gameState->getPlayMode() == PlayMode::PLAYING;
  };

  // FINISHED playmode condition
//   auto finishedCondition = []() {
//     auto gameState = AgentState::get<GameState>();
//     return gameState->getPlayMode() == PlayMode::FINISHED;
//   };

  if (!ignoreGameController)
  {
    // From pause to ready: press button
    auto pause2ReadyTransition = pauseState->newTransition();
    pause2ReadyTransition->condition = startButtonCondition;
    pause2ReadyTransition->onFire = [=]() { debugger->showReady(); };
    pause2ReadyTransition->childState = readyState;

    // From ready to set: game state changed
    auto ready2setTransition = readyState->newTransition();
    ready2setTransition->condition = setCondition;
    ready2setTransition->onFire = [=]() { debugger->showSet(); };
    ready2setTransition->childState = setState;

    // From set to play: game state changed TODO: go to their kickoff if it's theirs
    auto set2playingTransition = setState->newTransition();
    set2playingTransition->condition = playingCondition;
    set2playingTransition->onFire = [=]() { debugger->showPlaying(); };
    set2playingTransition->childState = playingState;

    // From set to penalized: penalized state
    auto set2penalizedTransition = setState->newTransition();
    set2penalizedTransition->condition = penaltyCondition;
    set2penalizedTransition->onFire = [=]() { debugger->showPenalized(); };
    set2penalizedTransition->childState = penalizedState;

    // From playing to penalized: penalized state
    auto playing2penalizedTransition = playingState->newTransition();
    playing2penalizedTransition->condition = penaltyCondition;
    playing2penalizedTransition->onFire = [=]() { debugger->showPenalized(); };
    playing2penalizedTransition->childState = penalizedState;

    // From penalized to set: no penalized state and game state
    auto penalized2setTransition = penalizedState->newTransition();
    penalized2setTransition->condition = [=] () {
      auto gameState = AgentState::get<GameState>();
      return noPenaltyCondition() && (gameState->getPlayMode() == PlayMode::SET);
    };
    penalized2setTransition->onFire = [=] () { debugger->showSet(); };
    penalized2setTransition->childState = setState;

    // From penalized to play: no penalized state and game state
    auto penalized2playingTransition = penalizedState->newTransition();
    penalized2playingTransition->condition = [=] () {
      auto gameState = AgentState::get<GameState>();
      return noPenaltyCondition() && (gameState->getPlayMode() == PlayMode::PLAYING);
    };
    penalized2playingTransition->onFire = [=] () { debugger->showPlaying(); };
    penalized2playingTransition->childState = playingState;

    // From play to paused: pause button
    auto play2pausedTransition = playingState->newTransition();
    play2pausedTransition->condition = startButtonCondition;
    play2pausedTransition->onFire = [=]() { debugger->showPaused(); };
    play2pausedTransition->childState = pauseState;
  } // !ignoreGameController
  else
  {
    auto pause2playingTransition = pauseState->newTransition();
    pause2playingTransition->condition = startButtonCondition;
    pause2playingTransition->onFire = [=] () { debugger->showPlaying(); };
    pause2playingTransition->childState = playingState;

    auto play2pausedTransition = playingState->newTransition();
    play2pausedTransition->condition = startButtonCondition;
    play2pausedTransition->onFire = [=]() { debugger->showPaused(); };
    play2pausedTransition->childState = pauseState;
  }

  //
  // ========== PLAYING ==========
  //

  // Goalie behavior
  if (uniformNumber == 1)
  {
    // Start state: stand up
    auto standUpState = playingFsm->newState("standup", {standup}, false/*endState*/, true/*startState*/);
  }
  // Player behavior
  else
  {
    // ---------- STATES ----------

    // Start state: stand up
    auto standUpState = playingFsm->newState("standup", {standup}, false/*endState*/, true/*startState*/);

    // State: stand and look look around
    auto lookForBallState = playingFsm->newState("lookforball", {stand, lookAround});

    // State: stand and look at ball
    auto lookAtBallState = playingFsm->newState("lookatball", {stand, lookAtBall});

    // State: approach and look at ball
    auto approachBallState = playingFsm->newState("approachball", {approachBall, lookAtBall});

    // State: look for goal
    auto lookForGoalState = playingFsm->newState("lookforgoal", {stand, lookAround});

    // State: stand and look at goal
    auto lookAtGoalState = playingFsm->newState("lookatgoal", {stand, lookAtGoal});

    // State: circle ball
    auto circleBallState = playingFsm->newState("circleball", {circleBall});

    // State: pre-kick look
    auto lookAtFeetState = playingFsm->newState("lookatfeet", {lookAtFeet});

    // State: left kick
    auto leftKickState = playingFsm->newState("leftkick", {leftKick});

    // State: right kick
    auto rightKickState = playingFsm->newState("rightkick", {rightKick});

    // ---------- TRANSITIONS ----------

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
    standUp2lookAround->childState = lookForBallState;

    // Transition: look at ball when visible
    auto lookAround2lookAtBall = lookForBallState->newTransition();
    lookAround2lookAtBall->condition = []() {
      return AgentState::get<CameraFrameState>()->isBallVisible();
    };
    lookAround2lookAtBall->childState = lookAtBallState;

    // Transition: look for ball if no longer seen
    auto lookAtBall2lookAround = lookAtBallState->newTransition();
    lookAtBall2lookAround->condition = ballLostCondition;
    lookAtBall2lookAround->childState = lookForBallState;

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
    approachBall2lookAround->childState = lookForBallState;

    // Transition: look for goal
    auto approachBall2lookForGoal = approachBallState->newTransition();
    approachBall2lookForGoal->condition = []() {
      auto ballObs = AgentState::get<AgentFrameState>()->getBallObservation();
      if (!ballObs)
        return false;

      return ((*ballObs)->head<2>().norm() < 0.1);
    };
    approachBall2lookForGoal->childState = lookForGoalState;

    // Transition: look at goal when visible
    auto lookForGoal2lookAtGoal = lookForGoalState->newTransition();
    lookForGoal2lookAtGoal->condition = []() {
      auto goalsObs = AgentState::get<AgentFrameState>()->getGoalObservations();
      return goalsObs.size() >= 2;
    };
    lookForGoal2lookAtGoal->childState = lookAtGoalState;

    // Transition: look for ball after too long
    auto lookForGoal2lookForBall = lookForGoalState->newTransition();
    lookForGoal2lookForBall->condition = [lookForGoal2lookForBall]() {
      return lookForGoal2lookForBall->parentState->secondsSinceStart() > 10;
    };
    lookForGoal2lookForBall->childState = lookForBallState;

    // Transition: if goal is seen right in front of us, start kick procedure
    auto lookAtGoal2lookDown = lookAtGoalState->newTransition();
    lookAtGoal2lookDown->condition = [lookAtGoal2lookDown]() {
      auto goalsObs = AgentState::get<AgentFrameState>()->getGoalObservations();

      if (lookAtGoal2lookDown->parentState->secondsSinceStart() > 2)
        return true;

      if (goalsObs.size() < 2)
        return false;

      double panAngle = MotionStatus::m_CurrentJoints.GetAngle(JointData::ID_HEAD_PAN);
      double panAngleRange = Head::GetInstance()->GetLeftLimitAngle();
      double panRatio = panAngle / panAngleRange;
      return abs(panRatio) < 0.2;
    };
    lookAtGoal2lookDown->childState = leftKickState;

    // Transition: seen goal, circle ball
    auto lookAtGoal2Circle = lookAtGoalState->newTransition();
    lookAtGoal2Circle->condition = [lookAtGoal2Circle]()
    {
      return lookAtGoal2Circle->parentState->secondsSinceStart() > 2.5;
    };
    lookAtGoal2Circle->childState = circleBallState;

    // Transition: stop circling
    auto circle2lookAtGoal = circleBallState->newTransition();
    circle2lookAtGoal->condition = [circle2lookAtGoal]()
    {
      double panAngle = MotionStatus::m_CurrentJoints.GetAngle(JointData::ID_HEAD_PAN);
      double panAngleRange = Head::GetInstance()->GetLeftLimitAngle();
      double panRatio = panAngle / panAngleRange;
      double circleDurationSeconds = panRatio * 0.4;
      return circle2lookAtGoal->parentState->secondsSinceStart() > circleDurationSeconds;
    };
    circle2lookAtGoal->childState = lookAtGoalState;

    auto kickLeft2lookForBall = leftKickState->newTransition();
    kickLeft2lookForBall->condition = [kickLeft2lookForBall]() {
      auto& os = kickLeft2lookForBall->parentState->options;
      return std::all_of(os.begin(), os.end(), [](OptionPtr o) { return o->hasTerminated(); });
    };
    kickLeft2lookForBall->childState = lookForBallState;

  } // uniformNumber != 1

  return tree;
}
