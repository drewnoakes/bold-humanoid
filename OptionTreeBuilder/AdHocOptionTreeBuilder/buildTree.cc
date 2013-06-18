#include "adhocoptiontreebuilder.ih"

#include "../../StateObject/BodyState/bodystate.hh"
#include "../../MotionModule/HeadModule/headmodule.hh"

unique_ptr<OptionTree> AdHocOptionTreeBuilder::buildTree(unsigned teamNumber,
                                                         unsigned uniformNumber,
                                                         bool ignoreGameController,
                                                         shared_ptr<Debugger> debugger,
                                                         shared_ptr<CameraModel> cameraModel,
                                                         shared_ptr<Ambulator> ambulator,
                                                         shared_ptr<ActionModule> actionModule,
                                                         shared_ptr<HeadModule> headModule,
                                                         shared_ptr<WalkModule> walkModule)
{
  unique_ptr<OptionTree> tree(new OptionTree());

  // Sit down action
  shared_ptr<Option> sit = make_shared<ActionOption>("sitdownaction", "sit down", actionModule);
  tree->addOption(sit);
  // Stand up action
  shared_ptr<Option> standup = make_shared<ActionOption>("standupaction", "stand up", actionModule);
  tree->addOption(standup);

  // Stop walking
  shared_ptr<Option> stopWalking = make_shared<StopWalking>("stopwalking", ambulator);
  tree->addOption(stopWalking);

  // Approach ball
  shared_ptr<Option> approachBall = make_shared<ApproachBall>("approachball", ambulator);
  tree->addOption(approachBall);

  // Circle around ball
  shared_ptr<Option> circleBall = make_shared<CircleBall>("circleball", ambulator, headModule);
  tree->addOption(circleBall);

  // Left kick
  shared_ptr<Option> leftKick = make_shared<ActionOption>("leftkickaction", "lk", actionModule);
  tree->addOption(leftKick);

  // Left kick
  shared_ptr<Option> rightKick = make_shared<ActionOption>("rightkickaction", "rk", actionModule);
  tree->addOption(rightKick);

  // Look around
  shared_ptr<Option> lookAround = make_shared<LookAround>("lookaround", headModule);
  tree->addOption(lookAround);

  // Look at ball
  shared_ptr<Option> lookAtBall = make_shared<LookAtBall>("lookatball", cameraModel, headModule);
  tree->addOption(lookAtBall);

  // Look at feet
  shared_ptr<Option> lookAtFeet = make_shared<LookAtFeet>("lookatfeet", headModule);
  tree->addOption(lookAtFeet);

  // Look at goal
  shared_ptr<Option> lookAtGoal = make_shared<LookAtGoal>("lookatgoal", cameraModel, headModule);
  tree->addOption(lookAtGoal);

  //Dive left
  shared_ptr<Option> leftdive = make_shared<ActionOption>("diveleftaction", "left_dive", actionModule);
  tree->addOption(leftdive);

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
  auto pauseState = winFsm->newState("pause", {sit,stopWalking}, false/*end state*/, ignoreGameController/*start state*/);

  auto unpausingState = winFsm->newState("unpausing", {standup});

  // State: ready
  auto readyState = winFsm->newState("ready", {stopWalking}, false/*end state*/, !ignoreGameController/* start state */);

  // State: set
  auto setState = winFsm->newState("set", {stopWalking});

  // State: beforeTheirKickoff
  auto beforeTheirKickoff = winFsm->newState("beforetheirkickoff", {stopWalking});

  // State: playing
  auto playingState = winFsm->newState("playing", {playingFsm});

  // State: penalized
  auto penalizedState = winFsm->newState("penalized", {stopWalking});


  // ---------- TRANSITIONS ----------

  auto startButtonCondition = []() {
    auto hw = AgentState::get<HardwareState>();
    if (!hw)
      return false;
    auto cm730 = hw->getCM730State();
    if (!cm730)
      return false;

    static bool lastState = false;
    if (lastState ^ cm730->isStartButtonPressed)
    {
      lastState = cm730->isStartButtonPressed;
      return lastState;
    }

    return false;
  };

  auto modeButtonCondition = []() {
    auto hw = AgentState::get<HardwareState>();
    if (!hw)
      return false;
    auto cm730 = hw->getCM730State();
    if (!cm730)
      return false;

    static bool lastState = false;
    if (lastState ^ cm730->isModeButtonPressed)
    {
      lastState = cm730->isModeButtonPressed;
      return lastState;
    }

    return false;
  };

  // Penalty condition
  auto penaltyCondition = [=]() {
    auto gameState = AgentState::get<GameState>();
    if (!gameState)
      return false;
    return gameState->teamInfo(teamNumber).getPlayer(uniformNumber).hasPenalty();
  };

  // No penalty condition
  // TODO: should be simply !penaltyCondition, but didn't work on first try
  auto noPenaltyCondition = [=]() {
    auto gameState = AgentState::get<GameState>();
    if (!gameState)
      return true;
    return !gameState->teamInfo(teamNumber).getPlayer(uniformNumber).hasPenalty();
  };

  auto makePlayModeCondition = [](PlayMode playMode, bool defaultValue)
  {
    return [=]() {
      auto gameState = AgentState::get<GameState>();
      if (!gameState)
        return defaultValue;
      return gameState->getPlayMode() == playMode;
    };
  };

  auto setCondition = makePlayModeCondition(PlayMode::SET, false);
  auto playingCondition = makePlayModeCondition(PlayMode::PLAYING, false);

  if (!ignoreGameController)
  {
    //
    // PAUSE BUTTON
    //

    auto pause2unpausingTransition = pauseState->newTransition();
    pause2unpausingTransition->condition = startButtonCondition;
    pause2unpausingTransition->childState = unpausingState;

    auto unpausing2playingTransition = unpausingState->newTransition();
    unpausing2playingTransition->condition = [unpausingState]() { return unpausingState->allOptionsTerminated(); };;
    unpausing2playingTransition->onFire = [=]() { debugger->showSet(); };
    unpausing2playingTransition->childState = setState;

    // From play to paused: pause button
    auto play2pausedTransition = playingState->newTransition("p2pStartBtn");
    play2pausedTransition->condition = startButtonCondition;
    play2pausedTransition->onFire = [=]() { debugger->showPaused(); };
    play2pausedTransition->childState = pauseState;

    //
    // MODE BUTTON
    //

    // TODO when in paused state, can the mode button somehow disable the motors?

    // From ready to set: button pressed
    auto ready2setManualTransition = readyState->newTransition("p2rCycleStateBtn");
    ready2setManualTransition->condition = modeButtonCondition;
    ready2setManualTransition->onFire = [=]() { debugger->showSet(); };
    ready2setManualTransition->childState = setState;

    // From set to penalised: button pressed
    auto set2PenalizedManualTransition = setState->newTransition("s2pCycleStateBtn");
    set2PenalizedManualTransition->condition = modeButtonCondition;
    set2PenalizedManualTransition->onFire = [=]() { debugger->showPenalized(); };
    set2PenalizedManualTransition->childState = penalizedState;

    // From penalized to play: button pressed
    auto penalized2PlayTransition = penalizedState->newTransition("p2pCycleStateBtn");
    penalized2PlayTransition->condition = modeButtonCondition;
    penalized2PlayTransition->onFire = [=]() { debugger->showPlaying(); };
    penalized2PlayTransition->childState = playingState;

    //
    // PLAY MODE TRANSITIONS -- GAME CONTROLLER
    //

    // From ready to set: game state changed
    auto ready2setTransition = readyState->newTransition("r2sGameController");
    ready2setTransition->condition = setCondition;
    ready2setTransition->onFire = [=]() { debugger->showSet(); };
    ready2setTransition->childState = setState;

    // From ready to playing: game state changed
    auto ready2playingTransition = readyState->newTransition("r2pGameController");
    ready2playingTransition->condition = playingCondition;
    ready2playingTransition->onFire = [=]() { debugger->showPlaying(); };
    ready2playingTransition->childState = playingState;

    // From set to penalized: penalized state
    auto set2penalizedTransition = setState->newTransition("s2pGameController");
    set2penalizedTransition->condition = penaltyCondition;
    set2penalizedTransition->onFire = [=]() { debugger->showPenalized(); };
    set2penalizedTransition->childState = penalizedState;

    // From set to play: game state changed TODO: go to their kickoff if it's theirs
    auto set2playingTransition = setState->newTransition("s2pGameController");
    set2playingTransition->condition = playingCondition;
    set2playingTransition->onFire = [=]() { debugger->showPlaying(); };
    set2playingTransition->childState = playingState;

    // From playing to penalized: penalized state
    auto playing2penalizedTransition = playingState->newTransition("p2pGameController");
    playing2penalizedTransition->condition = penaltyCondition;
    playing2penalizedTransition->onFire = [=]() { debugger->showPenalized(); };
    playing2penalizedTransition->childState = penalizedState;

    // From penalized to set: no penalized state and game state
    auto penalized2setTransition = penalizedState->newTransition("p2sGameController");
    penalized2setTransition->condition = [=]() {
      auto gameState = AgentState::get<GameState>();
      return gameState && noPenaltyCondition() && (gameState->getPlayMode() == PlayMode::SET);
    };
    penalized2setTransition->onFire = [=]() { debugger->showSet(); };
    penalized2setTransition->childState = setState;

    // From penalized to play: no penalized state and game state
    auto penalized2playingTransition = penalizedState->newTransition("p2pGameController");
    penalized2playingTransition->condition = [=]() {
      auto gameState = AgentState::get<GameState>();
      return gameState && noPenaltyCondition() && (gameState->getPlayMode() == PlayMode::PLAYING);
    };
    penalized2playingTransition->onFire = [=]() { debugger->showPlaying(); };
    penalized2playingTransition->childState = playingState;
  } // !ignoreGameController
  else
  {
    auto pause2unpausingTransition = pauseState->newTransition();
    pause2unpausingTransition->condition = startButtonCondition;
    pause2unpausingTransition->childState = unpausingState;

    auto unpausing2playingTransition = unpausingState->newTransition();
    unpausing2playingTransition->condition = [unpausingState]() { return unpausingState->allOptionsTerminated(); };;
    unpausing2playingTransition->onFire = [=]() { debugger->showPlaying(); };
    unpausing2playingTransition->childState = playingState;

    auto play2pausedTransition = playingState->newTransition();
    play2pausedTransition->condition = startButtonCondition;
    play2pausedTransition->onFire = [=]() { debugger->showPaused(); };
    play2pausedTransition->childState = pauseState;
  }

  ofstream winOut("win.dot");
  winOut << winFsm->toDot();

  //
  // ========== PLAYING ==========
  //

  // Goalie behavior
  if (uniformNumber == 1)
  {
    // Start state: stand up
    auto standUpState = playingFsm->newState("standup", {standup}, false/*endState*/, true/*startState*/);
  }
  // Penalty goalie behaviour
  else if (uniformNumber == 5)
  {
    // Start state: stand up
    auto standUpState = playingFsm->newState("standup", {standup}, false/*endState*/, true/*startState*/);
    // State: stand and look look around
    auto lookForBallState = playingFsm->newState("lookforball", {stopWalking, lookAround});

    // State: stand and look at ball
    auto lookAtBallState = playingFsm->newState("lookatball", {stopWalking, lookAtBall});

    // State: diving to the left
    auto leftDiveState = playingFsm->newState("leftdive", {leftdive});


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
    standUp2lookAround->condition = [standUpState]() {
      return standUpState->allOptionsTerminated();
    };
    standUp2lookAround->childState = lookForBallState;

    // Transition: look around -> look at ball when visible
    auto lookAround2lookAtBall = lookForBallState->newTransition();
    lookAround2lookAtBall->condition = []() {
      return AgentState::get<CameraFrameState>()->isBallVisible();
    };
    lookAround2lookAtBall->childState = lookAtBallState;

    // Transition: look at ball -> look for ball if no longer seen
    auto lookAtBall2lookAround = lookAtBallState->newTransition();
    lookAtBall2lookAround->condition = ballLostCondition;
    lookAtBall2lookAround->childState = lookForBallState;

    // Transition: look at ball -> diving to the left
    auto lookAtBall2leftDive = lookAtBallState->newTransition();
    lookAtBall2leftDive->condition = []() {
      auto ballObs = AgentState::get<AgentFrameState>()->getBallObservation();
      if (!ballObs)
        return false;
      Eigen::Vector3d ballPos = *ballObs;
      if (ballPos.y() > 1.0)
        return false;

      if (ballPos.x() > 0)
        return false;

      return true;
    };
    lookAtBall2leftDive->childState = leftDiveState;

    // Transition: dive to left -> back to look for goal
    auto leftDive2lookForBall = leftDiveState->newTransition();
    leftDive2lookForBall->condition = [leftDiveState]() {
      return leftDiveState->allOptionsTerminated();
    };
    leftDive2lookForBall->childState = lookForBallState;

  }
  // Player behavior
  else
  {
    // ---------- STATES ----------

    // Start state: stand up
    auto standUpState = playingFsm->newState("standup", {standup}, false/*endState*/, true/*startState*/);

    // State: stand and look look around
    auto lookForBallState = playingFsm->newState("lookforball", {stopWalking, lookAround});

    // State: circle around
    auto lookForBallCirclingState = playingFsm->newState("lookforballcircling", {circleBall});

    // State: stand and look at ball
    auto lookAtBallState = playingFsm->newState("lookatball", {stopWalking, lookAtBall});

    // State: approach and look at ball
    auto approachBallState = playingFsm->newState("approachball", {approachBall, lookAtBall});

    // State: look for goal
    auto lookForGoalState = playingFsm->newState("lookforgoal", {stopWalking, lookAround});

    // State: stand and look at goal
    auto lookAtGoalState = playingFsm->newState("lookatgoal", {stopWalking, lookAtGoal});

    // State: aim (transition state between looking at goal and either kicking or circling)
    auto aimState = playingFsm->newState("aim", {});

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
      static double lastSeen = 0;
      double t = Clock::getSeconds();
      if (AgentState::get<CameraFrameState>()->isBallVisible())
        lastSeen = t;
      return t - lastSeen > 1.0;
    };

    // Transition: into actual loop after stood up
    auto standUp2lookAround = standUpState->newTransition();
    standUp2lookAround->condition = [standUpState]() {
      return standUpState->allOptionsTerminated();
    };
    standUp2lookAround->childState = lookForBallState;

    // Transition: look around -> look at ball when visible
    auto lookAround2lookAtBall = lookForBallState->newTransition();
    lookAround2lookAtBall->condition = []() {
      return AgentState::get<CameraFrameState>()->isBallVisible();
    };
    lookAround2lookAtBall->childState = lookAtBallState;

    // Transition: look around -> circle around if takes too long
    auto lookForBall2lookForBallCircling = lookForBallState->newTransition();
    lookForBall2lookForBallCircling->condition = [lookForBallState]() {
      return lookForBallState->secondsSinceStart() > 10;
    };
    lookForBall2lookForBallCircling->childState = lookForBallCirclingState;

    // Transition: look for ball circling -> back to look for ball
    auto lookForBallCircling2lookForBall = lookForBallCirclingState->newTransition();
    lookForBallCircling2lookForBall->condition = [lookForBallCirclingState]() {
      return lookForBallCirclingState->secondsSinceStart() > 5;
    };
    lookForBallCircling2lookForBall->childState = lookForBallState;

    // Transition: look at ball -> look for ball if no longer seen
    auto lookAtBall2lookForBall = lookAtBallState->newTransition();
    lookAtBall2lookForBall->condition = ballLostCondition;
    lookAtBall2lookForBall->childState = lookForBallState;

    // Transition: look at ball -> approach ball
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

    // Transition: approach ball -> look for ball again if no longer seen
    auto approachBall2lookAround = approachBallState->newTransition("ballLost");
    approachBall2lookAround->condition = ballLostCondition;
    approachBall2lookAround->childState = lookForBallState;

    // Transition: approachball -> look for goal
    auto approachBall2lookForGoal = approachBallState->newTransition("closeToBall");
    approachBall2lookForGoal->condition = [playingFsm]() {
      auto ballObs = AgentState::get<AgentFrameState>()->getBallObservation();
      if (!ballObs)
        return false;

      return (ballObs->head<2>().norm() < playingFsm->getParam("approachBall.untilDistance", 0.05));
    };
    approachBall2lookForGoal->childState = lookForGoalState;

    // Transition: look for goal -> look at goal when visible
    auto lookForGoal2lookAtGoal = lookForGoalState->newTransition();
    lookForGoal2lookAtGoal->condition = []() {
      auto goalsObs = AgentState::get<AgentFrameState>()->getGoalObservations();
      return goalsObs.size() >= 2;
    };
    lookForGoal2lookAtGoal->childState = lookAtGoalState;

    // Transition: look for goal -> start kicking after too long
    auto lookForGoal2lookForBall = lookForGoalState->newTransition("lookTooLong");
    lookForGoal2lookForBall->condition = [lookForGoalState]() {
      return lookForGoalState->secondsSinceStart() > 5;
    };
    lookForGoal2lookForBall->childState = lookAtFeetState;

    // Transition: look at goal -> aim if seen goal for long enough
    auto lookAtGoal2aim = lookAtGoalState->newTransition();
    lookAtGoal2aim->condition = [lookAtGoalState]() {
      return lookAtGoalState->secondsSinceStart() > 0.5;
    };
    lookAtGoal2aim->childState = aimState;

    // Transition: aim -> if goal is seen right in front of us, start kick procedure
    auto aim2lookAtFeet = aimState->newTransition();
    aim2lookAtFeet->condition = [&headModule]() {
      auto body = AgentState::get<BodyState>();
      double panAngle = body->getHeadPanJoint()->angle;
      double panAngleRange = headModule->getLeftLimitRads();
      double panRatio = panAngle / panAngleRange;
//       cout << "panRatio: " << panRatio << endl;
      return fabs(panRatio) < 0.3;
    };
    aim2lookAtFeet->childState = lookAtFeetState;

    // Transition: look at goal -> circle ball
    auto aim2Circle = aimState->newTransition();
    aim2Circle->condition = []()
    {
      return true;
    };
    aim2Circle->childState = circleBallState;

    // Transition: circle -> stop circling and look for goal again
    auto circle2lookForGoal = circleBallState->newTransition();
    circle2lookForGoal->condition = [circleBallState,&headModule]()
    {
      auto body = AgentState::get<BodyState>();
      double panAngle = body->getHeadPanJoint()->angle;
      double panAngleRange = headModule->getLeftLimitRads();
      double panRatio = panAngle / panAngleRange;
      double circleDurationSeconds = fabs(panRatio) * 3;
//       cout << "circleDuration: " << circleDurationSeconds << endl;
//       cout << "Seconds since start: " << circleBallState->secondsSinceStart() << endl;
      return circleBallState->secondsSinceStart() > circleDurationSeconds;
    };
    circle2lookForGoal->childState = lookForGoalState;

    // Transition: look at feet -> kick left
    auto lookAtFeet2kickLeft = lookAtFeetState->newTransition();
    lookAtFeet2kickLeft->condition = [lookAtFeetState] () {
      if (lookAtFeetState->secondsSinceStart() < 1)
        return false;

      // Wait until we're finished looking down
      if (!lookAtFeetState->allOptionsTerminated())
        return false;

      auto ballObs = AgentState::get<AgentFrameState>()->getBallObservation();

      if (ballObs && ballObs->y() <= 0.2 && ballObs->x() < 0)
      {
        cout << "[lookAtFeet2kickLeft] Kicking with left foot when ball at (" << ballObs->x() << "," << ballObs->y() << ")" << endl;
        return true;
      }
      return false;
    };
    lookAtFeet2kickLeft->childState = leftKickState;

    // Transition: look at feet -> kick right
    auto lookAtFeet2kickRight = lookAtFeetState->newTransition();
    lookAtFeet2kickRight->condition = [lookAtFeetState] () {
      if (lookAtFeetState->secondsSinceStart() < 1)
        return false;

      // Wait until we're finished looking down
      if (!lookAtFeetState->allOptionsTerminated())
        return false;

      auto ballObs = AgentState::get<AgentFrameState>()->getBallObservation();

      if (ballObs && ballObs->y() <= 0.2 && ballObs->x() > 0)
      {
        cout << "[lookAtFeet2kickRight] Kicking with right foot when ball at (" << ballObs->x() << "," << ballObs->y() << ")" << endl;
        return true;
      }
      return false;
    };
    lookAtFeet2kickRight->childState = rightKickState;

    // Transition: look at feet -> look for ball
    auto lookAtFeet2lookForBall = lookAtFeetState->newTransition();
    lookAtFeet2lookForBall->condition = [lookAtFeetState] () {
      if (lookAtFeetState->secondsSinceStart() < 1)
        return false;

      // Wait until we're finished looking down
      return lookAtFeetState->allOptionsTerminated();
    };
    lookAtFeet2lookForBall->childState = lookForBallState;

    // Transition: kick left -> look for ball
    auto kickLeft2lookForBall = leftKickState->newTransition();
    kickLeft2lookForBall->condition = [leftKickState]() {
      return leftKickState->allOptionsTerminated();
    };
    kickLeft2lookForBall->childState = lookForBallState;

    // Transition: kick right -> look for ball
    auto kickRight2lookForBall = rightKickState->newTransition();
    kickRight2lookForBall->condition = [rightKickState]() {
      return rightKickState->allOptionsTerminated();
    };
    kickRight2lookForBall->childState = lookForBallState;
  } // uniformNumber != 1

  ofstream playingOut("playing.dot");
  playingOut << playingFsm->toDot();

  return tree;
}
