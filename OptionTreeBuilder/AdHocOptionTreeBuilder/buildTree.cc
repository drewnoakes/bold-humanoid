#include "adhocoptiontreebuilder.ih"

#include "../../Agent/agent.hh"
#include "../../Ambulator/ambulator.hh"
#include "../../DataStreamer/datastreamer.hh"
#include "../../StateObject/BodyState/bodystate.hh"
#include "../../MotionModule/HeadModule/headmodule.hh"
#include "../../util/conditionals.hh"
#include "../../util/Range.hh"

unique_ptr<OptionTree> AdHocOptionTreeBuilder::buildTree(Agent* agent)
{
  const unsigned UNUM_GOALIE = 1;
  const unsigned UNUM_GOALIE_PENALTY = 5;

  unsigned uniformNumber   = agent->getUniformNumber();
  unsigned teamNumber      = agent->getTeamNumber();

  auto const& debugger           = agent->getDebugger();
  auto const& cameraModel        = agent->getCameraModel();
  auto const& ambulator          = agent->getAmbulator();
  auto const& motionScriptModule = agent->getMotionScriptModule();
  auto const& headModule         = agent->getHeadModule();
  auto const& fallDetector       = agent->getFallDetector();

  // TODO any / all / true functions

  // GENERAL FUNCTIONS

  auto secondsSinceStart = [](double seconds, shared_ptr<FSMState> state)
  {
    return [state,seconds]() { return state->secondsSinceStart() >= seconds; };
  };

  auto startButtonPressed = []()
  {
    auto hw = AgentState::get<HardwareState>();
    if (!hw)
      return false;
    auto const& cm730 = hw->getCM730State();

    static bool lastState = false;
    if (lastState ^ cm730.isStartButtonPressed)
    {
      lastState = cm730.isStartButtonPressed;
      return lastState;
    }

    return false;
  };

  auto modeButtonPressed = []()
  {
    auto hw = AgentState::get<HardwareState>();
    if (!hw)
      return false;
    auto const& cm730 = hw->getCM730State();

    static bool lastState = false;
    if (lastState ^ cm730.isModeButtonPressed)
    {
      lastState = cm730.isModeButtonPressed;
      return lastState;
    }

    return false;
  };

  auto ballVisibleCondition = []()
  {
    return AgentState::get<CameraFrameState>()->isBallVisible();
  };

  // TODO review this one-size-fits-all approach on a case-by-case basis below
  auto ballFoundConditionFactory = [ballVisibleCondition]() { return trueForMillis(1000, ballVisibleCondition); };
  auto ballLostConditionFactory = [ballVisibleCondition]() { return trueForMillis(1000, negate(ballVisibleCondition)); };

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

  auto isAgentShutdownRequested = changedTo(true, [agent]() { return agent->isStopRequested(); });

  // BUILD TREE

  unique_ptr<OptionTree> tree(new OptionTree());

  // OPTIONS

  auto sit = tree->addOption(make_shared<MotionScriptOption>("sitDownScript", motionScriptModule, "./motionscripts/sit-down.json"));
  auto sitArmsBack = tree->addOption(make_shared<MotionScriptOption>("sitDownScript", motionScriptModule, "./motionscripts/sit-down-arms-back.json"));
  auto standUp = tree->addOption(make_shared<MotionScriptOption>("standUpScript", motionScriptModule, "./motionscripts/stand-ready-upright.json"));
  auto forwardGetUp = tree->addOption(make_shared<MotionScriptOption>("forwardGetUpScript", motionScriptModule, "./motionscripts/get-up-from-front.json"));
  auto backwardGetUp = tree->addOption(make_shared<MotionScriptOption>("backwardGetUpScript", motionScriptModule, "./motionscripts/get-up-from-back.json"));
  auto leftDive = tree->addOption(make_shared<MotionScriptOption>("diveleftScript", motionScriptModule, "./motionscripts/dive-left.json"));
  auto rightDive = tree->addOption(make_shared<MotionScriptOption>("diverightScript", motionScriptModule, "./motionscripts/dive-right.json"));
  auto bigStepLeft = tree->addOption(make_shared<MotionScriptOption>("bigStepLeftScript", motionScriptModule, "./motionscripts/step-left-big.json"));
  auto bigStepRight = tree->addOption(make_shared<MotionScriptOption>("bigStepRightScript", motionScriptModule, "./motionscripts/step-right-big.json"));
  auto leftKick = tree->addOption(make_shared<MotionScriptOption>("leftKickScript", motionScriptModule, "./motionscripts/kick-left.json"));
  auto rightKick = tree->addOption(make_shared<MotionScriptOption>("rightKickScript", motionScriptModule, "./motionscripts/kick-right.json"));

  auto stopWalking = tree->addOption(make_shared<StopWalking>("stopWalking", ambulator));
  auto approachBall = tree->addOption(make_shared<ApproachBall>("approachBall", ambulator));
  auto circleBall = tree->addOption(make_shared<CircleBall>("circleBall", ambulator, headModule));
  auto lookAround = tree->addOption(make_shared<LookAround>("lookAround", headModule, 100.0));
  auto lookAroundNarrow = tree->addOption(make_shared<LookAround>("lookAroundNarrow", headModule, 45.0));
  auto lookAtBall = tree->addOption(make_shared<LookAtBall>("lookAtBall", cameraModel, headModule));
  auto lookAtFeet = tree->addOption(make_shared<LookAtFeet>("lookAtFeet", headModule));
  auto lookAtGoal = tree->addOption(make_shared<LookAtGoal>("lookAtGoal", cameraModel, headModule));

  // FSMs

  auto winFsm = tree->addOption(make_shared<FSMOption>(agent->getVoice(), "win"), /*isRoot*/true);

  auto playingFsm = tree->addOption(make_shared<FSMOption>(agent->getVoice(), "playing"));

  //
  // ========== WIN ==========
  //

  auto startUpState = winFsm->newState("startUp", {sit}, false/*end state*/, true/* start state */);
  auto readyState = winFsm->newState("ready", {stopWalking});
  auto pausing1State = winFsm->newState("pausing1", {stopWalking});
  auto pausing2State = winFsm->newState("pausing2", {sit});
  auto pausedState = winFsm->newState("paused", {});
  auto unpausingState = winFsm->newState("unpausing", {standUp});
  auto setState = winFsm->newState("set", {stopWalking});
  auto beforeTheirKickoff = winFsm->newState("beforeTheirKickOff", {stopWalking});
  auto playingState = winFsm->newState("playing", {playingFsm});
  auto penalizedState = winFsm->newState("penalized", {stopWalking});
  auto forwardGetUpState = winFsm->newState("forwardGetUp", {forwardGetUp});
  auto backwardGetUpState = winFsm->newState("backwardGetUp", {backwardGetUp});
  auto stopWalkingForShutdownState = winFsm->newState("stopWalkingForShutdown", {stopWalking});
  auto sitForShutdownState = winFsm->newState("sitForShutdown", {sitArmsBack});
  auto stopAgentAndExitState = winFsm->newState("stopAgentAndExit", {});

  // TODO control that allows announcing states entered, provided speech queue isn't too long (off by default)


  readyState->onEnter = [debugger,headModule]() { debugger->showReady(); headModule->moveToHome(); };
  setState->onEnter = [debugger,headModule]() { debugger->showSet(); headModule->moveToHome(); };
  playingState->onEnter = [debugger]() { debugger->showPlaying(); };
  penalizedState->onEnter = [debugger,headModule]() { debugger->showPenalized(); headModule->moveToHome(); };
  pausedState->onEnter = [debugger]() { debugger->showPaused(); };
  pausing1State->onEnter = [debugger,headModule]() { debugger->showPaused(); headModule->moveToHome(); };
  stopAgentAndExitState->onEnter = [agent]() { agent->stop(); };

  //
  // START UP
  //

  startUpState
    ->transitionTo(readyState)
    ->whenTerminated();

  //
  // PAUSE BUTTON
  //

  pausedState
    ->transitionTo(unpausingState)
    ->when(startButtonPressed);

  unpausingState
    ->transitionTo(setState)
    ->whenTerminated();

  playingState
    ->transitionTo(pausing1State)
    ->when(startButtonPressed);

  pausing1State
    ->transitionTo(pausing2State)
    ->when(negate(isWalking));

  pausing2State
    ->transitionTo(pausedState)
    ->whenTerminated();

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
    ->whenTerminated();

  backwardGetUpState
    ->transitionTo(playingState)
    ->whenTerminated();

  //
  // SHUTDOWN
  //

  // TODO express this sequence more elegantly

  winFsm
    ->wildcardTransitionTo(stopWalkingForShutdownState)
    ->when(isAgentShutdownRequested);

  stopWalkingForShutdownState
    ->transitionTo(sitForShutdownState)
    ->when(negate(isWalking)); // TODO why can't this be whenTerminated() -- doesn't seem to work (here and in other places)

  sitForShutdownState
    ->transitionTo(stopAgentAndExitState)
    ->whenTerminated();

  //
  // ========== PLAYING ==========
  //

  if (uniformNumber == UNUM_GOALIE)
  {
    // Goalie behaviour for normal game play

    // TODO test this further
    // TODO add logic to kick ball away from goal if close to keeper

    auto standUpState = playingFsm->newState("standUp", {standUp}, false/*endState*/, true/*startState*/);
    auto lookForBallState = playingFsm->newState("lookForBall", {stopWalking, lookAround});
    auto lookAtBallState = playingFsm->newState("lookAtBall", {stopWalking, lookAtBall});
    auto bigStepLeftState = playingFsm->newState("bigStepLeft", {bigStepLeft});
    auto bigStepRightState = playingFsm->newState("bigStepRight", {bigStepRight});

    standUpState
      ->transitionTo(lookForBallState)
      ->whenTerminated();

    lookForBallState
      ->transitionTo(lookAtBallState)
      ->when(ballFoundConditionFactory);

    lookAtBallState
      ->transitionTo(lookForBallState)
      ->when(ballLostConditionFactory);

    lookAtBallState
      ->transitionTo(bigStepLeftState)
      ->when([]()
      {
        return trueForMillis(1000, []()
        {
          auto ball = AgentState::get<AgentFrameState>()->getBallObservation();
          return ball && Range<double>(0.75, 1.5).contains(ball->y()) && Range<double>(-0.75, -0.3).contains(ball->x());
        });
      });

    lookAtBallState
      ->transitionTo(bigStepRightState)
      ->when([]()
      {
        return trueForMillis(1000, []()
        {
          auto ball = AgentState::get<AgentFrameState>()->getBallObservation();
          return ball && Range<double>(0.75, 1.5).contains(ball->y()) && Range<double>(0.3, 0.75).contains(ball->x());
        });
      });

    bigStepLeftState
      ->transitionTo(lookForBallState)
      ->whenTerminated();

    bigStepRightState
      ->transitionTo(lookForBallState)
      ->whenTerminated();

  }
  else if (uniformNumber == UNUM_GOALIE_PENALTY)
  {
    // Goalie behaviour during penalties

    auto standUpState = playingFsm->newState("standUp", {standUp}, false/*endState*/, true/*startState*/);
    auto lookForBallState = playingFsm->newState("lookForBall", {stopWalking, lookAroundNarrow});
    auto lookAtBallState = playingFsm->newState("lookAtBall", {stopWalking, lookAtBall});
    auto leftDiveState = playingFsm->newState("leftDive", {leftDive});
    auto rightDiveState = playingFsm->newState("rightDive", {rightDive});

    standUpState
      ->transitionTo(lookForBallState)
      ->whenTerminated();

    lookForBallState
      ->transitionTo(lookAtBallState)
      ->when(ballFoundConditionFactory);

    lookAtBallState
      ->transitionTo(lookForBallState)
      ->when(ballLostConditionFactory);

    lookAtBallState
      ->transitionTo(leftDiveState)
      ->when([]()
      {
        return trueForMillis(100, []()
        {
          auto ball = AgentState::get<AgentFrameState>()->getBallObservation();
          return ball && ball->y() < 1.0 && ball->x() < -0.1;
        });
      });

    lookAtBallState
      ->transitionTo(rightDiveState)
      ->when([]()
      {
        return trueForMillis(100, []()
        {
          auto ball = AgentState::get<AgentFrameState>()->getBallObservation();
          return ball && ball->y() < 1.0 && ball->x() > 0.1;
        });
      });

    leftDiveState
      ->transitionTo(lookForBallState)
      ->whenTerminated();

    rightDiveState
      ->transitionTo(lookForBallState)
      ->whenTerminated();
  }
  else
  {
    //
    // PLAYER BEHAVIOR
    //

    auto standUpState = playingFsm->newState("standUp", {standUp}, false/*endState*/, true/*startState*/);
    auto lookForBallState = playingFsm->newState("lookForBall", {stopWalking, lookAround});
    auto circleToFindLostBallState = playingFsm->newState("lookForBallCircling", {circleBall});
    auto lookAtBallState = playingFsm->newState("lookAtBall", {stopWalking, lookAtBall});
    auto approachBallState = playingFsm->newState("approachBall", {approachBall, lookAtBall});
    auto lookForGoalState = playingFsm->newState("lookForGoal", {stopWalking, lookAround});
    auto lookAtGoalState = playingFsm->newState("lookAtGoal", {stopWalking, lookAtGoal});
    auto aimState = playingFsm->newState("aim", {});
    auto circleBallState = playingFsm->newState("circleBall", {circleBall});
    auto lookAtFeetState = playingFsm->newState("lookAtFeet", {lookAtFeet});
    auto leftKickState = playingFsm->newState("leftKick", {leftKick});
    auto rightKickState = playingFsm->newState("rightKick", {rightKick});

    standUpState
      ->transitionTo(lookForBallState)
      ->whenTerminated();

    lookForBallState
      ->transitionTo(lookAtBallState)
      ->when([ballVisibleCondition]() { return stepUpDownThreshold(5, ballVisibleCondition); });

    // walk a circle if we don't find the ball within some time limit
    lookForBallState
      ->transitionTo(circleToFindLostBallState)
      ->when(secondsSinceStart(8, lookForBallState));

    // after 5 seconds of circling, look for the ball again
    circleToFindLostBallState
      ->transitionTo(lookForBallState)
      ->when(secondsSinceStart(5, circleToFindLostBallState));

    lookAtBallState
      ->transitionTo(lookForBallState)
      ->when(ballLostConditionFactory);

    // start approaching the ball when we have the confidence that it's really there
    // TODO this doesn't filter the ball position, so may be misled by jitter
    lookAtBallState
      ->transitionTo(approachBallState)
      ->when([ballVisibleCondition]() { return stepUpDownThreshold(10, ballVisibleCondition); });

    approachBallState
      ->transitionTo(lookForBallState)
      ->when(ballLostConditionFactory);

    // stop walking to ball once we're close enough
    approachBallState
      ->transitionTo(lookForGoalState)
      ->when([playingFsm]()
      {
        // Approach ball until we're within a given distance
        auto ballObs = AgentState::get<AgentFrameState>()->getBallObservation();
        return ballObs && (ballObs->head<2>().norm() < Config::getValue<double>("options.approach-ball.stop-distance"));
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
      ->when([]()
      {
        double panAngle = AgentState::get<BodyState>()->getJoint(JointId::HEAD_PAN)->angleRads;
        // TODO this angular limit in config
        return fabs(Math::radToDeg(panAngle)) < 25.0;
      });

    // circle immediately, if goal is not in front (prior transition didn't fire)
    aimState
      ->transitionTo(circleBallState)
      ->when([]() { return true; });

    // control duration of ball circling
    circleBallState
      ->transitionTo(lookForGoalState)
      ->when([circleBallState,headModule,secondsSinceStart]()
      {
        // TODO break dependency upon pan limit
        double panAngle = AgentState::get<BodyState>()->getJoint(JointId::HEAD_PAN)->angleRads;
        double panAngleRange = headModule->getLeftLimitRads();
        double panRatio = panAngle / panAngleRange;
        double circleDurationSeconds = fabs(panRatio) * 4.5;
        log::info("circleBallState")
            << "circleDurationSeconds=" << circleDurationSeconds
            << " secondsSinceStart=" << circleBallState->secondsSinceStart()
            << " panRatio=" << panRatio
            << " panAngle=" << panAngle
            << " leftLimitDegs=" << headModule->getLeftLimitDegs();

        return secondsSinceStart(circleDurationSeconds, circleBallState);
      });

    // TODO if ball too central, step to left/right slightly, or use different kick

    lookAtFeetState
      ->transitionTo(leftKickState)
      ->when([lookAtFeetState]()
      {
        if (lookAtFeetState->secondsSinceStart() < 1)
          return false;

        // Wait until we've finished looking down
        if (!lookAtFeetState->allOptionsTerminated())
          return false;

        auto ballObs = AgentState::get<AgentFrameState>()->getBallObservation();

        if (ballObs && ballObs->y() <= 0.2 && ballObs->x() < 0)
        {
          log::info("lookAtFeet2kickLeft") << "Kicking with left foot when ball at (" << ballObs->x() << "," << ballObs->y() << ")";
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

        // Wait until we've finished looking down
        if (!lookAtFeetState->allOptionsTerminated())
          return false;

        auto ballObs = AgentState::get<AgentFrameState>()->getBallObservation();

        if (ballObs && ballObs->y() <= 0.2 && ballObs->x() > 0)
        {
          log::info("lookAtFeet2kickRight") << "Kicking with right foot when ball at (" << ballObs->x() << "," << ballObs->y() << ")";
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

        // Wait until we've finished looking down
        return lookAtFeetState->allOptionsTerminated();
      });

    leftKickState
      ->transitionTo(lookForBallState)
      ->whenTerminated();

    rightKickState
      ->transitionTo(lookForBallState)
      ->whenTerminated();
  } // uniformNumber != 1

  // Write out DOT files for visualisation of states and transitions

  ofstream winOut("win.dot");
  winOut << winFsm->toDot();

  ofstream playingOut("playing.dot");
  playingOut << playingFsm->toDot();

  return tree;
}
