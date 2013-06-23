#include "adhocoptiontreebuilder.ih"

#include "../../Agent/agent.hh"
#include "../../Ambulator/ambulator.hh"
#include "../../CM730/cm730.hh"
#include "../../StateObject/BodyState/bodystate.hh"
#include "../../MotionModule/HeadModule/headmodule.hh"
#include "../../util/conditionals.hh"
#include "../../util/Range.hh"

unique_ptr<OptionTree> AdHocOptionTreeBuilder::buildTree(unsigned teamNumber,
                                                         unsigned uniformNumber,
                                                         Agent* agent,
                                                         shared_ptr<Debugger> debugger,
                                                         shared_ptr<CameraModel> cameraModel,
                                                         shared_ptr<Ambulator> ambulator,
                                                         shared_ptr<ActionModule> actionModule,
                                                         shared_ptr<HeadModule> headModule,
                                                         shared_ptr<WalkModule> walkModule,
                                                         shared_ptr<FallDetector> fallDetector)
{
  const unsigned UNUM_GOALIE = 1;
  const unsigned UNUM_GOALIE_PENALTY = 5;

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

  auto isWalking = [ambulator]() { return ambulator->isRunning(); };

  auto hasFallenForward = [fallDetector]() { return fallDetector->getFallenState() == FallState::FORWARD; };

  auto hasFallenBackward = [fallDetector]() { return fallDetector->getFallenState() == FallState::BACKWARD; };

  auto isAgentShutdownRequested = [agent]() { return agent->isStopRequested(); };

  // BUILD TREE

  unique_ptr<OptionTree> tree(new OptionTree());

  // OPTIONS

  // TODO correct capitalisation of options

  auto sit = tree->addOption(make_shared<ActionOption>("sitdownaction", "sit down", actionModule));
  auto standup = tree->addOption(make_shared<ActionOption>("standupaction", "stand up", actionModule));
  auto forwardgetup = tree->addOption(make_shared<ActionOption>("forwardgetupaction", ActionPage::ForwardGetUp, actionModule));
  auto backwardgetup = tree->addOption(make_shared<ActionOption>("backwardgetupaction", ActionPage::BackwardGetUp, actionModule));
  auto stopWalking = tree->addOption(make_shared<StopWalking>("stopwalking", ambulator));
  auto approachBall = tree->addOption(make_shared<ApproachBall>("approachball", ambulator));
  auto circleBall = tree->addOption(make_shared<CircleBall>("circleball", ambulator, headModule));
  auto leftKick = tree->addOption(make_shared<ActionOption>("leftkickaction", "lk", actionModule));
  auto rightKick = tree->addOption(make_shared<ActionOption>("rightkickaction", "rk", actionModule));
  auto lookAround = tree->addOption(make_shared<LookAround>("lookaround", headModule, 100.0));
  auto lookAroundNarrow = tree->addOption(make_shared<LookAround>("lookaroundnarrow", headModule, 45.0));
  auto lookAtBall = tree->addOption(make_shared<LookAtBall>("lookatball", cameraModel, headModule));
  auto lookAtFeet = tree->addOption(make_shared<LookAtFeet>("lookatfeet", headModule));
  auto lookAtGoal = tree->addOption(make_shared<LookAtGoal>("lookatgoal", cameraModel, headModule));
  auto leftdive = tree->addOption(make_shared<ActionOption>("diveleftaction", "left_dive", actionModule));
  auto rightdive = tree->addOption(make_shared<ActionOption>("diverightaction", "right_dive", actionModule));
  auto bigStepLeft = tree->addOption(make_shared<ActionOption>("bigstepleftaction", "big-step-l", actionModule));
  auto bigStepRight = tree->addOption(make_shared<ActionOption>("bigsteprightaction", "big-step-r", actionModule));

  // FSMs

  auto winFsm = tree->addOption(make_shared<FSMOption>("win"), /*isRoot*/true);

  auto playingFsm = tree->addOption(make_shared<FSMOption>("playing"));

  //
  // ========== WIN ==========
  //

  auto pausingState = winFsm->newState("pausing", {stopWalking});
  auto pausedState = winFsm->newState("paused", {sit});
  auto unpausingState = winFsm->newState("unpausing", {standup});
  auto readyState = winFsm->newState("ready", {stopWalking}, false/*end state*/, true/* start state */);
  auto setState = winFsm->newState("set", {stopWalking});
  auto beforeTheirKickoff = winFsm->newState("beforetheirkickoff", {stopWalking});
  auto playingState = winFsm->newState("playing", {playingFsm});
  auto penalizedState = winFsm->newState("penalized", {stopWalking});
  auto forwardGetUpState = winFsm->newState("forwardgetup", {forwardgetup});
  auto backwardGetUpState = winFsm->newState("backwardgetup", {backwardgetup});
  auto stopWalkingForShutdownState = winFsm->newState("stopWalkingForShutdown", {stopWalking});
  auto sitForShutdownState = winFsm->newState("sitForShutdown", {sit});
  auto stopAgentAndExitState = winFsm->newState("stopAgentAndExit", {});

  readyState->onEnter = [debugger,headModule]() { debugger->showReady(); headModule->moveToHome(); };
  setState->onEnter = [debugger,headModule]() { debugger->showSet(); headModule->moveToHome(); };
  playingState->onEnter = [debugger]() { debugger->showPlaying(); };
  penalizedState->onEnter = [debugger,headModule]() { debugger->showPenalized(); headModule->moveToHome(); };
  pausedState->onEnter = [debugger]() { debugger->showPaused(); };
  pausingState->onEnter = [debugger,headModule]() { debugger->showPaused(); headModule->moveToHome(); };
  stopAgentAndExitState->onEnter = [agent]() { agent->getCM730()->torqueEnable(false); agent->stop(); };

  //
  // PAUSE BUTTON
  //

  pausedState
    ->transitionTo(unpausingState)
    ->when(startButtonPressed);

  unpausingState
    ->transitionTo(setState)
    ->when(hasTerminated(unpausingState));

  playingState
    ->transitionTo(pausingState)
    ->when(startButtonPressed);

  pausingState
    ->transitionTo(pausedState)
    ->when(negate(isWalking));

  //
  // PLAY MODE BUTTON
  //

  readyState
    ->transitionTo(setState)
    ->when(modeButtonPressed);

  setState
    ->transitionTo(penalizedState)
    ->when(modeButtonPressed);

  penalizedState
    ->transitionTo(playingState)
    ->when(modeButtonPressed);

  //
  // GAME CONTROLLER PLAY MODE
  //

  readyState
    ->transitionTo(setState)
    ->when(isSetPlayMode);

  readyState
    ->transitionTo(playingState)
    ->when(isPlayingPlayMode);

  setState
    ->transitionTo(penalizedState)
    ->when(isPenalised);

  setState
    ->transitionTo(playingState)
    ->when(isPlayingPlayMode);

  playingState
    ->transitionTo(penalizedState)
    ->when(isPenalised);

  playingState
    ->transitionTo(readyState)
    ->when(nonPenalisedPlayMode(PlayMode::READY));

  playingState
    ->transitionTo(setState)
    ->when(nonPenalisedPlayMode(PlayMode::SET));

  penalizedState
    ->transitionTo(setState)
    ->when(nonPenalisedPlayMode(PlayMode::SET));

  penalizedState
    ->transitionTo(playingState)
    ->when(nonPenalisedPlayMode(PlayMode::PLAYING));

  // FALLEN TRANSITIONS
  playingState
    ->transitionTo(forwardGetUpState)
    ->when(hasFallenForward);

  playingState
    ->transitionTo(backwardGetUpState)
    ->when(hasFallenBackward);

  forwardGetUpState
    ->transitionTo(playingState)
    ->when(hasTerminated(forwardGetUpState));

  backwardGetUpState
    ->transitionTo(playingState)
    ->when(hasTerminated(backwardGetUpState));

  //
  // SHUTDOWN
  //

  // TODO express this sequence more eligantly
  // TODO wildcard winFSM.* -> stopWalkingForShutdown (not just from playingState)

  playingState
    ->transitionTo(stopWalkingForShutdownState)
    ->when(isAgentShutdownRequested);

  stopWalkingForShutdownState
    ->transitionTo(sitForShutdownState)
    ->when(negate(isWalking));

  sitForShutdownState
    ->transitionTo(stopAgentAndExitState)
    ->when(hasTerminated(sitForShutdownState));

  //
  // ========== PLAYING ==========
  //

  if (uniformNumber == UNUM_GOALIE)
  {
    // Goalie behaviour for normal game play

    // TODO test this further
    // TODO add logic to kick ball away from goal if close to keeper

    auto standUpState = playingFsm->newState("standup", {standup}, false/*endState*/, true/*startState*/);
    auto lookForBallState = playingFsm->newState("lookforball", {stopWalking, lookAround});
    auto lookAtBallState = playingFsm->newState("lookatball", {stopWalking, lookAtBall});
    auto bigStepLeftState = playingFsm->newState("bigStepLeft", {bigStepLeft});
    auto bigStepRightState = playingFsm->newState("bigStepRight", {bigStepRight});

    standUpState->transitionTo(lookForBallState)
      ->when(hasTerminated(standUpState));

    lookForBallState->transitionTo(lookAtBallState)
      ->when(ballVisibleCondition);

    lookAtBallState->transitionTo(lookForBallState)
      ->when(ballLostCondition);

    lookAtBallState->transitionTo(bigStepLeftState)
      ->when(oneShot([]()
      {
        return trueForMillis(1000, []()
        {
          auto ball = AgentState::get<AgentFrameState>()->getBallObservation();
          return ball && Range<double>(0.75, 1.5).contains(ball->y()) && Range<double>(-0.75, -0.3).contains(ball->x());
        });
      }));

    lookAtBallState->transitionTo(bigStepRightState)
      ->when(oneShot([]()
      {
        return trueForMillis(1000, []()
        {
          auto ball = AgentState::get<AgentFrameState>()->getBallObservation();
          return ball && Range<double>(0.75, 1.5).contains(ball->y()) && Range<double>(0.3, 0.75).contains(ball->x());
        });
      }));

    bigStepLeftState->transitionTo(lookForBallState)
      ->when(hasTerminated(bigStepLeftState));

    bigStepRightState->transitionTo(lookForBallState)
      ->when(hasTerminated(bigStepRightState));

  }
  else if (uniformNumber == UNUM_GOALIE_PENALTY)
  {
    // Goalie behaviour during penalties

    auto standUpState = playingFsm->newState("standup", {standup}, false/*endState*/, true/*startState*/);
    auto lookForBallState = playingFsm->newState("lookforball", {stopWalking, lookAroundNarrow});
    auto lookAtBallState = playingFsm->newState("lookatball", {stopWalking, lookAtBall});
    auto leftDiveState = playingFsm->newState("leftdive", {leftdive});
    auto rightDiveState = playingFsm->newState("rightdive", {rightdive});

    standUpState->transitionTo(lookForBallState)
      ->when(hasTerminated(standUpState));

    lookForBallState->transitionTo(lookAtBallState)
      ->when(ballVisibleCondition);

    lookAtBallState->transitionTo(lookForBallState)
      ->when(ballLostCondition);

    lookAtBallState->transitionTo(leftDiveState)
      ->when(oneShot([]()
      {
        return trueForMillis(100, []()
        {
          auto ball = AgentState::get<AgentFrameState>()->getBallObservation();
          return ball && ball->y() < 1.0 && ball->x() < -0.1;
        });
      }));

    lookAtBallState->transitionTo(rightDiveState)
      ->when(oneShot([]()
      {
        return trueForMillis(100, []()
        {
          auto ball = AgentState::get<AgentFrameState>()->getBallObservation();
          return ball && ball->y() < 1.0 && ball->x() > 0.1;
        });
      }));

    leftDiveState->transitionTo(lookForBallState)
      ->when(hasTerminated(leftDiveState));

    rightDiveState->transitionTo(lookForBallState)
      ->when(hasTerminated(rightDiveState));
  }
  else
  {
    //
    // PLAYER BEHAVIOR
    //

    auto standUpState = playingFsm->newState("standup", {standup}, false/*endState*/, true/*startState*/);
    auto lookForBallState = playingFsm->newState("lookforball", {stopWalking, lookAround});
    auto circleToFindLostBallState = playingFsm->newState("lookforballcircling", {circleBall});
    auto lookAtBallState = playingFsm->newState("lookatball", {stopWalking, lookAtBall});
    auto approachBallState = playingFsm->newState("approachball", {approachBall, lookAtBall});
    auto lookForGoalState = playingFsm->newState("lookforgoal", {stopWalking, lookAround});
    auto lookAtGoalState = playingFsm->newState("lookatgoal", {stopWalking, lookAtGoal});
    auto aimState = playingFsm->newState("aim", {});
    auto circleBallState = playingFsm->newState("circleball", {circleBall});
    auto lookAtFeetState = playingFsm->newState("lookatfeet", {lookAtFeet});
    auto leftKickState = playingFsm->newState("leftkick", {leftKick});
    auto rightKickState = playingFsm->newState("rightkick", {rightKick});

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
    // TODO this doesn't filter the ball position, so may be misled by jitter
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
        // TODO create and use 'all' operator
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

  // Write out DOT files for visualisation of states and transitions

  ofstream winOut("win.dot");
  winOut << winFsm->toDot();

  ofstream playingOut("playing.dot");
  playingOut << playingFsm->toDot();

  return tree;
}
