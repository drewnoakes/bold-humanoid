#include "adhocoptiontreebuilder.ih"

#include "../../StateObject/BodyState/bodystate.hh"
#include "../../MotionModule/HeadModule/headmodule.hh"
#include "../../util/conditionals.hh"

unique_ptr<OptionTree> AdHocOptionTreeBuilder::buildTree(unsigned teamNumber,
                                                         unsigned uniformNumber,
                                                         shared_ptr<Debugger> debugger,
                                                         shared_ptr<CameraModel> cameraModel,
                                                         shared_ptr<Ambulator> ambulator,
                                                         shared_ptr<ActionModule> actionModule,
                                                         shared_ptr<HeadModule> headModule,
                                                         shared_ptr<WalkModule> walkModule)
{
  // GENERAL FUNCTIONS

  auto secondsSinceStart = [](double seconds, FSMStatePtr state)
  {
    return [state,seconds]() { return state->secondsSinceStart() >= seconds; };
  };

  // TODO uses of hasTerminated are all on the state that is being observed -- use a better fluent API

  auto hasTerminated = [](FSMStatePtr state)
  {
    return [state]() { return state->allOptionsTerminated(); };
  };

  auto startButtonPressed = []()
  {
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

  auto modeButtonPressed = []()
  {
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

  auto ballVisibleCondition = []()
  {
    return AgentState::get<CameraFrameState>()->isBallVisible();
  };

  // TODO merge these two? do they have to be different? look at usages

//   auto ballLostCondition = oneShot([ballVisibleCondition]() { return isRepeated(10, negate(ballVisibleCondition)); });

  auto ballLostCondition = oneShot([ballVisibleCondition]() { return trueForMillis(1000, negate(ballVisibleCondition)); });

//   auto ballLostCondition = trueForMillis(1000, negate(ballVisibleCondition));

//   auto ballLostCondition = []()
//   {
//     static double lastSeen = 0;
//     double t = Clock::getSeconds();
//     if (AgentState::get<CameraFrameState>()->isBallVisible())
//       lastSeen = t;
//     return t - lastSeen > 1.0;
//   };

  auto isPenalised = [=]()
  {
    auto gameState = AgentState::get<GameState>();
    return gameState && gameState->teamInfo(teamNumber).getPlayer(uniformNumber).hasPenalty();
  };

  auto isNotPenalised = [=]()
  {
    auto gameState = AgentState::get<GameState>();
    return gameState && !gameState->teamInfo(teamNumber).getPlayer(uniformNumber).hasPenalty();
  };

  auto nonPenalisedPlayMode = [isNotPenalised](PlayMode playMode)
  {
    return [isNotPenalised,playMode]()
    {
      auto gameState = AgentState::get<GameState>();
      return gameState && isNotPenalised() && gameState->getPlayMode() == playMode;
    };
  };

  auto isPlayMode = [](PlayMode playMode, bool defaultValue)
  {
    return [=]()
    {
      auto gameState = AgentState::get<GameState>();
      if (!gameState)
        return defaultValue;
      return gameState->getPlayMode() == playMode;
    };
  };

  auto isSetPlayMode = isPlayMode(PlayMode::SET, false);
  auto isPlayingPlayMode = isPlayMode(PlayMode::PLAYING, false);

  // BUILD TREE

  unique_ptr<OptionTree> tree(new OptionTree());

  // OPTIONS

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

  auto pausingState = winFsm->newState("pausing", {stopWalking});

  auto pausedState = winFsm->newState("paused", {sit});

  auto unpausingState = winFsm->newState("unpausing", {standup});

  auto readyState = winFsm->newState("ready", {stopWalking}, false/*end state*/, true/* start state */);

  auto setState = winFsm->newState("set", {stopWalking});

  auto beforeTheirKickoff = winFsm->newState("beforetheirkickoff", {stopWalking});

  auto playingState = winFsm->newState("playing", {playingFsm});

  auto penalizedState = winFsm->newState("penalized", {stopWalking});

  // ---------- TRANSITIONS ----------

  //
  // PAUSE BUTTON
  //

  // TODO all these debugger->show* calls might better be modelled on the states themselves as entry actions

  pausedState
    ->transitionTo(unpausingState)
    ->when(startButtonPressed);

  unpausingState
    ->transitionTo(setState)
    ->when(hasTerminated(unpausingState))
    ->notify([=]() { debugger->showSet(); });

  playingState
    ->transitionTo(pausingState)
    ->when(startButtonPressed)
    ->notify([=]() { debugger->showPaused(); });

  //
  // MODE BUTTON
  //

  // TODO when in paused state, can the mode button somehow disable the motors?

  readyState
    ->transitionTo(setState)
    ->when(modeButtonPressed)
    ->notify([=]() { debugger->showSet(); });

  setState
    ->transitionTo(penalizedState)
    ->when(modeButtonPressed)
    ->notify([=]() { debugger->showPenalized(); });

  penalizedState
    ->transitionTo(playingState)
    ->when(modeButtonPressed)
    ->notify([=]() { debugger->showPlaying(); });

  //
  // PLAY MODE TRANSITIONS -- GAME CONTROLLER
  //

  readyState
    ->transitionTo(setState)
    ->when(isSetPlayMode)
    ->notify([=]() { debugger->showSet(); });

  readyState
    ->transitionTo(playingState)
    ->when(isPlayingPlayMode)
    ->notify([=]() { debugger->showPlaying(); });

  setState
    ->transitionTo(penalizedState)
    ->when(isPenalised)
    ->notify([=]() { debugger->showPenalized(); });

  setState
    ->transitionTo(playingState)
    ->when(isPlayingPlayMode)
    ->notify([=]() { debugger->showPlaying(); });

  playingState
    ->transitionTo(penalizedState)
    ->when(isPenalised)
    ->notify([=]() { debugger->showPenalized(); });

  penalizedState
    ->transitionTo(setState)
    ->when(nonPenalisedPlayMode(PlayMode::SET))
    ->notify([=]() { debugger->showSet(); });

  penalizedState
    ->transitionTo(playingState)
    ->when(nonPenalisedPlayMode(PlayMode::PLAYING))
    ->notify([=]() { debugger->showPlaying(); });

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
    auto standUpState = playingFsm->newState("standup", {standup}, false/*endState*/, true/*startState*/);

    auto lookForBallState = playingFsm->newState("lookforball", {stopWalking, lookAround});

    auto lookAtBallState = playingFsm->newState("lookatball", {stopWalking, lookAtBall});

    auto leftDiveState = playingFsm->newState("leftdive", {leftdive});

    // ---------- TRANSITIONS ----------

    standUpState->transitionTo(lookForBallState)
                ->when(hasTerminated(standUpState));

    lookForBallState->transitionTo(lookAtBallState)
                    ->when(ballVisibleCondition);

    lookAtBallState->transitionTo(lookForBallState)
                  ->when(ballLostCondition);

    lookAtBallState->transitionTo(leftDiveState)
                   ->when([]()
                   {
                     auto ball = AgentState::get<AgentFrameState>()->getBallObservation();
                     return ball && ball->y() < 1.0 && ball->x() < 0;
                   });

    // TODO introduce rightDiveState
//     lookAtBallState->transitionTo(rightDiveState)
//                    ->when([]()
//                    {
//                      auto ball = AgentState::get<AgentFrameState>()->getBallObservation();
//                      return ball && ball->y() < 1.0 && ball->x() > 0;
//                    });

    leftDiveState->transitionTo(lookForBallState)
                 ->when(hasTerminated(leftDiveState));
  }
  else
  {
    //
    // PLAYER BEHAVIOR
    //

    // ---------- STATES ----------

    auto standUpState = playingFsm->newState("standup", {standup}, false/*endState*/, true/*startState*/);

    auto lookForBallState = playingFsm->newState("lookforball", {stopWalking, lookAround});

    auto circleToFindLostBallState = playingFsm->newState("lookforballcircling", {circleBall});

    auto lookAtBallState = playingFsm->newState("lookatball", {stopWalking, lookAtBall});

    auto approachBallState = playingFsm->newState("approachball", {approachBall, lookAtBall});

    auto lookForGoalState = playingFsm->newState("lookforgoal", {stopWalking, lookAround});

    auto lookAtGoalState = playingFsm->newState("lookatgoal", {stopWalking, lookAtGoal});

    // transition state between looking at goal and either kicking or circling
    auto aimState = playingFsm->newState("aim", {});

    auto circleBallState = playingFsm->newState("circleball", {circleBall});

    auto lookAtFeetState = playingFsm->newState("lookatfeet", {lookAtFeet});

    auto leftKickState = playingFsm->newState("leftkick", {leftKick});

    auto rightKickState = playingFsm->newState("rightkick", {rightKick});

    // ---------- TRANSITIONS ----------

    standUpState
      ->transitionTo(lookForBallState)
      ->when(hasTerminated(standUpState));

    lookForBallState
      ->transitionTo(lookAtBallState)
      ->when(ballVisibleCondition);

    // walk a circle if we don't find the ball within 10 seconds
    lookForBallState
      ->transitionTo(circleToFindLostBallState)
      ->when(secondsSinceStart(10, lookForBallState));

    // after 5 seconds of circling, look for the ball again
    circleToFindLostBallState
      ->transitionTo(lookForBallState)
      ->when(secondsSinceStart(5, circleToFindLostBallState));

    lookAtBallState
      ->transitionTo(lookForBallState)
      ->when(ballLostCondition);

    // start approaching the ball when we have the confidence that it's really there
    // TODO this doesn't filter the ball position, so may be mislead by jitter
    lookAtBallState
      ->transitionTo(approachBallState)
      ->when(oneShot([ballVisibleCondition]() { return stepUpDownThreshold(10, ballVisibleCondition); }));

    approachBallState
      ->transitionTo(lookForBallState)
      ->when(ballLostCondition);

    // stop walking to ball once we're close enough
    approachBallState
      ->transitionTo(lookForGoalState)
      ->when([playingFsm]()
      {
        // Approach ball until we're within a given distance
        auto ballObs = AgentState::get<AgentFrameState>()->getBallObservation();
        return ballObs && (ballObs->head<2>().norm() < playingFsm->getParam("approachBall.untilDistance", 0.075));
      });

    lookForGoalState
      ->transitionTo(lookAtGoalState)
      ->when([]()
      {
        auto goalsObs = AgentState::get<AgentFrameState>()->getGoalObservations();
        return goalsObs.size() >= 2;
      });

    // limit how long we will look for the goal
    lookForGoalState
      ->transitionTo(lookAtFeetState)
      ->when(secondsSinceStart(7, lookForGoalState));

    lookAtGoalState
      ->transitionTo(aimState)
      ->when(secondsSinceStart(0.5, lookAtGoalState));

    // start kick procedure if goal is in front of us
    aimState
      ->transitionTo(lookAtFeetState)
      ->when([&headModule]()
      {
        double panAngle = AgentState::get<BodyState>()->getHeadPanJoint()->angle;
        double panAngleRange = headModule->getLeftLimitRads();
        double panRatio = panAngle / panAngleRange;
        return fabs(panRatio) < 0.3;
      });

    // circle immediately, if goal is not in front (prior transition didn't fire)
    aimState
      ->transitionTo(circleBallState)
      ->when([]() { return true; });

    // control duration of ball circling
    circleBallState
      ->transitionTo(lookForGoalState)
      ->when([circleBallState,&headModule]()
      {
        // BUG head keeps moving during this state, making the below logic incorrect
        double panAngle = AgentState::get<BodyState>()->getHeadPanJoint()->angle;
        double panAngleRange = headModule->getLeftLimitRads();
        double panRatio = panAngle / panAngleRange;
        double circleDurationSeconds = fabs(panRatio) * 4.5;
        cout << "[circleBallState] circleDurationSeconds=" << circleDurationSeconds
             << " secondsSinceStart=" << circleBallState->secondsSinceStart()
             << " panRatio=" << panRatio
             << " panAngle=" << panAngle
             << " leftLimitDegs=" << headModule->getLeftLimitDegs() << endl;
        return circleBallState->secondsSinceStart() > circleDurationSeconds;
      });

    lookAtFeetState
      ->transitionTo(leftKickState)
      ->when([lookAtFeetState]()
      {
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
      });

    lookAtFeetState
      ->transitionTo(rightKickState)
      ->when([lookAtFeetState]()
      {
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
      });

    lookAtFeetState
      ->transitionTo(lookForBallState)
      ->when([lookAtFeetState]()
      {
        // TODO use 'all' operator
        if (lookAtFeetState->secondsSinceStart() < 1)
          return false;

        // Wait until we're finished looking down
        return lookAtFeetState->allOptionsTerminated();
      });

    leftKickState
      ->transitionTo(lookForBallState)
      ->when(hasTerminated(leftKickState));

    rightKickState
      ->transitionTo(lookForBallState)
      ->when(hasTerminated(rightKickState));
  } // uniformNumber != 1

  ofstream playingOut("playing.dot");
  playingOut << playingFsm->toDot();

  return tree;
}
