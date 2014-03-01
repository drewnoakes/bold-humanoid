#include "adhocoptiontreebuilder.ih"

#include "../../Agent/agent.hh"
#include "../../Ambulator/ambulator.hh"
#include "../../DataStreamer/datastreamer.hh"
#include "../../StateObject/BodyState/bodystate.hh"
#include "../../MotionModule/HeadModule/headmodule.hh"
#include "../../util/conditionals.hh"
#include "../../util/Range.hh"

shared_ptr<OptionTree> AdHocOptionTreeBuilder::buildTree(Agent* agent)
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

  auto tree = make_shared<OptionTree>();

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
  auto lookAroundNarrow = tree->addOption(make_shared<LookAround>("lookAroundNarrow", headModule, 45.0));
  auto lookForGoal = tree->addOption(make_shared<LookAround>("lookForGoal", headModule, 100.0, []() { return 1 - 0.33*AgentState::get<CameraFrameState>()->getGoalObservationCount(); }));
  auto lookForBall = tree->addOption(make_shared<LookAround>("lookForBall", headModule, 135.0, []() { return AgentState::get<CameraFrameState>()->isBallVisible() ? 0.25 : 1.0; }));
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
  auto playingState = winFsm->newState("playing", {playingFsm});
  auto penalizedState = winFsm->newState("penalized", {stopWalking});
  auto forwardGetUpState = winFsm->newState("forwardGetUp", {forwardGetUp});
  auto backwardGetUpState = winFsm->newState("backwardGetUp", {backwardGetUp});
  auto stopWalkingForShutdownState = winFsm->newState("stopWalkingForShutdown", {stopWalking});
  auto sitForShutdownState = winFsm->newState("sitForShutdown", {sitArmsBack});
  auto stopAgentAndExitState = winFsm->newState("stopAgentAndExit", {});

  readyState->onEnter.connect([debugger,headModule]() { debugger->showReady(); headModule->moveToHome(); });
  setState->onEnter.connect([debugger,headModule]() { debugger->showSet(); headModule->moveToHome(); });
  playingState->onEnter.connect([debugger]() { debugger->showPlaying(); });
  penalizedState->onEnter.connect([debugger,headModule]() { debugger->showPenalized(); headModule->moveToHome(); });
  pausedState->onEnter.connect([debugger]() { debugger->showPaused(); });
  pausing1State->onEnter.connect([debugger,headModule]() { debugger->showPaused(); headModule->moveToHome(); });
  stopAgentAndExitState->onEnter.connect([agent]() { agent->stop(); });

  //
  // START UP
  //

  startUpState
    ->transitionTo(readyState, "initialised")
    ->whenTerminated();

  //
  // PAUSE BUTTON
  //

  pausedState
    ->transitionTo(unpausingState, "button2")
    ->when(startButtonPressed);

  unpausingState
    ->transitionTo(setState, "done")
    ->whenTerminated();

  playingState
    ->transitionTo(pausing1State, "button2")
    ->when(startButtonPressed);

  pausing1State
    ->transitionTo(pausing2State, "stop-walk")
    ->when(negate(isWalking));

  pausing2State
    ->transitionTo(pausedState, "done")
    ->whenTerminated();

  //
  // PLAY MODE BUTTON
  //

  readyState
    ->transitionTo(setState, "button1")
    ->when(modeButtonPressed);

  setState
    ->transitionTo(penalizedState, "button1")
    ->when(modeButtonPressed);

  penalizedState
    ->transitionTo(playingState, "button1")
    ->when(modeButtonPressed);

  //
  // GAME CONTROLLER PLAY MODE
  //

  readyState
    ->transitionTo(setState, "gc-set")
    ->when(isSetPlayMode);

  readyState
    ->transitionTo(playingState, "gc-playing")
    ->when(isPlayingPlayMode);

  setState
    ->transitionTo(penalizedState, "gc-penalised")
    ->when(isPenalised);

  setState
    ->transitionTo(playingState, "gc-playing")
    ->when(isPlayingPlayMode);

  playingState
    ->transitionTo(penalizedState, "gc-penalised")
    ->when(isPenalised);

  playingState
    ->transitionTo(readyState, "gc-ready")
    ->when(nonPenalisedPlayMode(PlayMode::READY));

  playingState
    ->transitionTo(setState, "gc-set")
    ->when(nonPenalisedPlayMode(PlayMode::SET));

  penalizedState
    ->transitionTo(setState, "gc-unpenalised")
    ->when(nonPenalisedPlayMode(PlayMode::SET));

  penalizedState
    ->transitionTo(playingState, "gc-unpenalised")
    ->when(nonPenalisedPlayMode(PlayMode::PLAYING));

  // FALLEN TRANSITIONS
  playingState
    ->transitionTo(forwardGetUpState, "fall-fwd")
    ->when(hasFallenForward);

  playingState
    ->transitionTo(backwardGetUpState, "fall-back")
    ->when(hasFallenBackward);

  forwardGetUpState
    ->transitionTo(playingState, "done")
    ->whenTerminated();

  backwardGetUpState
    ->transitionTo(playingState, "done")
    ->whenTerminated();

  //
  // SHUTDOWN
  //

  // TODO express this sequence more elegantly

  winFsm
    ->wildcardTransitionTo(stopWalkingForShutdownState, "shutdown-request")
    ->when(isAgentShutdownRequested);

  stopWalkingForShutdownState
    ->transitionTo(sitForShutdownState, "stopped")
    ->when(negate(isWalking)); // TODO why can't this be whenTerminated() -- doesn't seem to work (here and in other places)

  sitForShutdownState
    ->transitionTo(stopAgentAndExitState, "sitting")
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
    auto lookForBallState = playingFsm->newState("lookForBall", {stopWalking, lookForBall});
    auto lookAtBallState = playingFsm->newState("lookAtBall", {stopWalking, lookAtBall});
    auto bigStepLeftState = playingFsm->newState("bigStepLeft", {bigStepLeft});
    auto bigStepRightState = playingFsm->newState("bigStepRight", {bigStepRight});

    standUpState
      ->transitionTo(lookForBallState, "standing")
      ->whenTerminated();

    lookForBallState
      ->transitionTo(lookAtBallState, "found")
      ->when(ballFoundConditionFactory);

    lookAtBallState
      ->transitionTo(lookForBallState, "lost")
      ->when(ballLostConditionFactory);

    lookAtBallState
      ->transitionTo(bigStepLeftState, "ball-left")
      ->when([]()
      {
        return trueForMillis(1000, []()
        {
          auto ball = AgentState::get<AgentFrameState>()->getBallObservation();
          return ball && Range<double>(0.75, 1.5).contains(ball->y()) && Range<double>(-0.75, -0.3).contains(ball->x());
        });
      });

    lookAtBallState
      ->transitionTo(bigStepRightState, "ball-right")
      ->when([]()
      {
        return trueForMillis(1000, []()
        {
          auto ball = AgentState::get<AgentFrameState>()->getBallObservation();
          return ball && Range<double>(0.75, 1.5).contains(ball->y()) && Range<double>(0.3, 0.75).contains(ball->x());
        });
      });

    bigStepLeftState
      ->transitionTo(lookForBallState, "done")
      ->whenTerminated();

    bigStepRightState
      ->transitionTo(lookForBallState, "done")
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
      ->transitionTo(lookForBallState, "standing")
      ->whenTerminated();

    lookForBallState
      ->transitionTo(lookAtBallState, "found")
      ->when(ballFoundConditionFactory);

    lookAtBallState
      ->transitionTo(lookForBallState, "lost-ball")
      ->when(ballLostConditionFactory);

    lookAtBallState
      ->transitionTo(leftDiveState, "ball-left")
      ->when([]()
      {
        return trueForMillis(100, []()
        {
          auto ball = AgentState::get<AgentFrameState>()->getBallObservation();
          return ball && ball->y() < 1.0 && ball->x() < -0.1;
        });
      });

    lookAtBallState
      ->transitionTo(rightDiveState, "ball-right")
      ->when([]()
      {
        return trueForMillis(100, []()
        {
          auto ball = AgentState::get<AgentFrameState>()->getBallObservation();
          return ball && ball->y() < 1.0 && ball->x() > 0.1;
        });
      });

    leftDiveState
      ->transitionTo(lookForBallState, "done")
      ->whenTerminated();

    rightDiveState
      ->transitionTo(lookForBallState, "done")
      ->whenTerminated();
  }
  else
  {
    //
    // PLAYER BEHAVIOR
    //

    auto standUpState = playingFsm->newState("standUp", {standUp}, false/*endState*/, true/*startState*/);
    auto lookForBallState = playingFsm->newState("lookForBall", {stopWalking, lookForBall});
    auto circleToFindLostBallState = playingFsm->newState("lookForBallCircling", {circleBall});
    auto lookAtBallState = playingFsm->newState("lookAtBall", {stopWalking, lookAtBall});
    auto approachBallState = playingFsm->newState("approachBall", {approachBall, lookAtBall});
    auto lookForGoalState = playingFsm->newState("lookForGoal", {stopWalking, lookForGoal});
    auto lookAtGoalState = playingFsm->newState("lookAtGoal", {stopWalking, lookAtGoal});
    auto aimState = playingFsm->newState("aim", {});
    auto circleBallState = playingFsm->newState("circleBall", {circleBall});
    auto lookAtFeetState = playingFsm->newState("lookAtFeet", {lookAtFeet});
    auto leftKickState = playingFsm->newState("leftKick", {leftKick});
    auto rightKickState = playingFsm->newState("rightKick", {rightKick});

    lookAtFeetState->onEnter.connect([lookAtFeet]() { lookAtFeet->reset(); });

    standUpState
      ->transitionTo(lookForBallState, "standing")
      ->whenTerminated();

    lookForBallState
      ->transitionTo(lookAtBallState, "found")
      ->when([ballVisibleCondition]() { return stepUpDownThreshold(5, ballVisibleCondition); });

    // walk a circle if we don't find the ball within some time limit
    lookForBallState
      ->transitionTo(circleToFindLostBallState, "lost-ball-long")
      ->when(secondsSinceStart(8, lookForBallState));

    // after 5 seconds of circling, look for the ball again
    circleToFindLostBallState
      ->transitionTo(lookForBallState, "done")
      ->when(secondsSinceStart(5, circleToFindLostBallState));

    lookAtBallState
      ->transitionTo(lookForBallState, "lost-ball")
      ->when(ballLostConditionFactory);

    // start approaching the ball when we have the confidence that it's really there
    // TODO this doesn't filter the ball position, so may be misled by jitter
    lookAtBallState
      ->transitionTo(approachBallState, "found")
      ->when([ballVisibleCondition]() { return stepUpDownThreshold(10, ballVisibleCondition); });

    approachBallState
      ->transitionTo(lookForBallState, "lost-ball")
      ->when(ballLostConditionFactory);

    // stop walking to ball once we're close enough
    approachBallState
      ->transitionTo(lookForGoalState, "near-ball")
      ->when([playingFsm]()
      {
        // Approach ball until we're within a given distance
        // TODO use filtered ball position
        auto ballObs = AgentState::get<AgentFrameState>()->getBallObservation();
        static auto stoppingDistance = Config::getSetting<double>("options.approach-ball.stop-distance");
        return ballObs && (ballObs->head<2>().norm() < stoppingDistance->getValue());
      });

    lookForGoalState
      ->transitionTo(lookAtGoalState, "see-both-goals")
      ->when([]()
      {
        // TODO use the localiser here rather than requiring both posts to be in frame
        auto goalsObs = AgentState::get<AgentFrameState>()->getGoalObservations();
        return goalsObs.size() >= 2;
      });

    // if we notice the ball has gone while looking for the goal, quit
    lookForGoalState
      ->transitionTo(lookForBallState, "ball-gone")
      ->when([]()
      {
        // If the ball is far away, then stop looking for the goal
        // TODO use filtered ball position
        return stepUpDownThreshold(6, []()
        {
          auto ballObs = AgentState::get<AgentFrameState>()->getBallObservation();
          static auto stoppingDistance = Config::getSetting<double>("options.approach-ball.stop-distance");
          // TODO introduce another setting 'max-kick-distance' instead of this scaling of the stop distance
          return ballObs && ballObs->head<2>().norm() > 2 * stoppingDistance->getValue();
        });
      });

    // limit how long we will look for the goal
    lookForGoalState
      ->transitionTo(lookAtFeetState, "give-up")
      ->when(secondsSinceStart(7, lookForGoalState));

    lookAtGoalState
      ->transitionTo(aimState, "confident")
      ->when(secondsSinceStart(0.5, lookAtGoalState));

    // start kick procedure if goal is in front of us
    aimState
      ->transitionTo(lookAtFeetState, "square-to-goal")
      ->when([]()
      {
        double panAngle = AgentState::get<BodyState>(StateTime::CameraImage)->getJoint(JointId::HEAD_PAN)->angleRads;
        // TODO this angular limit in config
        return fabs(Math::radToDeg(panAngle)) < 25.0;
      });

    // circle immediately, if goal is not in front (prior transition didn't fire)
    aimState
      ->transitionTo(circleBallState, "goal-at-angle")
      ->when([]() { return true; });

    // control duration of ball circling
    circleBallState
      ->transitionTo(lookAtGoalState, "done")
      ->when([circleBallState,headModule,secondsSinceStart]()
      {
        // TODO break dependency upon pan limit
        double panAngle = AgentState::get<BodyState>(StateTime::CameraImage)->getJoint(JointId::HEAD_PAN)->angleRads;
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
      ->transitionTo(leftKickState, "ball-left")
      ->when([lookAtFeet,lookAtFeetState]()
      {
        if (lookAtFeetState->secondsSinceStart() < 1)
          return false;

        // Wait until we've finished looking down
        if (!lookAtFeetState->allOptionsTerminated())
          return false;

        if (lookAtFeet->hasPosition())
        {
          auto ballPos = lookAtFeet->getAverageBallPositionAgentFrame();
          if (ballPos.y() <= 0.2 && ballPos.x() < 0)
          {
            log::info("lookAtFeet2kickLeft") << "Kicking with left foot when ball at (" << ballPos.x() << "," << ballPos.y() << ")";
            return true;
          }
        }
        return false;
      });

    lookAtFeetState
      ->transitionTo(rightKickState, "ball-right")
      ->when([lookAtFeet,lookAtFeetState]()
      {
        if (lookAtFeetState->secondsSinceStart() < 1)
          return false;

        // Wait until we've finished looking down
        if (!lookAtFeetState->allOptionsTerminated())
          return false;

        if (lookAtFeet->hasPosition())
        {
          auto ballPos = lookAtFeet->getAverageBallPositionAgentFrame();
          if (ballPos.y() <= 0.2 && ballPos.x() < 0)
          {
            log::info("lookAtFeet2kickRight") << "Kicking with right foot when ball at (" << ballPos.x() << "," << ballPos.y() << ")";
            return true;
          }
        }
        return false;
      });

    lookAtFeetState
      ->transitionTo(lookForBallState, "ball-gone")
      ->when([lookAtFeetState]()
      {
        // TODO create and use 'all' operator
        if (lookAtFeetState->secondsSinceStart() < 1)
          return false;

        // Wait until we've finished looking down
        return lookAtFeetState->allOptionsTerminated();
      });

    leftKickState
      ->transitionTo(lookForBallState, "done")
      ->whenTerminated();

    rightKickState
      ->transitionTo(lookForBallState, "done")
      ->whenTerminated();
  } // uniformNumber != 1

  // Write out DOT files for visualisation of states and transitions

  ofstream winOut("win.dot");
  winOut << winFsm->toDot();

  ofstream playingOut("playing.dot");
  playingOut << playingFsm->toDot();

  return tree;
}
