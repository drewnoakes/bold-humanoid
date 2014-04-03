#include "adhocoptiontreebuilder.ih"

#include "../../Agent/agent.hh"
#include "../../Ambulator/ambulator.hh"
#include "../../BehaviourControl/behaviourcontrol.hh"
#include "../../DataStreamer/datastreamer.hh"
#include "../../StateObject/BodyState/bodystate.hh"
#include "../../MotionModule/HeadModule/headmodule.hh"
#include "../../Option/DispatchOption/dispatchoption.hh"
#include "../../util/conditionals.hh"
#include "../../util/Range.hh"

using namespace robocup;

//
// Utility functions for setting status flags as we enter states
//

auto setPlayerActivityInStates = [](Agent* agent, PlayerActivity activity, std::vector<shared_ptr<FSMState>> states)
{
  for (shared_ptr<FSMState>& state : states)
    state->onEnter.connect([agent,activity]() { agent->getBehaviourControl()->setPlayerActivity(activity); });
};

auto setPlayerStatusInStates = [](Agent* agent, PlayerStatus status, std::vector<shared_ptr<FSMState>> states)
{
  for (shared_ptr<FSMState>& state : states)
    state->onEnter.connect([agent,status]() { agent->getBehaviourControl()->setPlayerStatus(status); });
};


//
// CONDITIONALS
//

// TODO any / all / true functions

// GENERAL FUNCTIONS

auto startButtonPressed = []()
{
  auto hw = State::get<HardwareState>();
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
  auto hw = State::get<HardwareState>();
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
  return State::get<CameraFrameState>()->isBallVisible();
};

auto ballTooFarToKick = []()
{
  // TODO use filtered ball position
  auto ballObs = State::get<AgentFrameState>()->getBallObservation();
  static auto maxKickDistance = Config::getSetting<double>("kick.max-ball-distance");
  return ballObs && ballObs->head<2>().norm() > maxKickDistance->getValue();
};

// TODO review this one-size-fits-all approach on a case-by-case basis below
auto ballFoundConditionFactory = []() { return trueForMillis(1000, ballVisibleCondition); };
auto ballLostConditionFactory = []() { return trueForMillis(1000, bold::negate(ballVisibleCondition)); };

auto isPlayMode = [](PlayMode playMode, bool defaultValue)
{
  return [playMode,defaultValue]()
  {
    auto gameState = State::get<GameState>();
    if (!gameState)
      return defaultValue;
    return gameState->getPlayMode() == playMode;
  };
};

auto isSetPlayMode = isPlayMode(PlayMode::SET, false);
auto isPlayingPlayMode = isPlayMode(PlayMode::PLAYING, false);

///////////////////////////////////////////////////////////////////////////////

//
// FSM BUILDERS
//

shared_ptr<FSMOption> buildStrikerFsm(Agent* agent, shared_ptr<OptionTree> tree)
{
  auto standUp = make_shared<MotionScriptOption>("standUpScript", agent->getMotionScriptModule(), "./motionscripts/stand-ready-upright.json");
  auto leftKick = make_shared<MotionScriptOption>("leftKickScript", agent->getMotionScriptModule(), "./motionscripts/kick-left.json");
  auto rightKick = make_shared<MotionScriptOption>("rightKickScript", agent->getMotionScriptModule(), "./motionscripts/kick-right.json");
  auto stopWalking = make_shared<StopWalking>("stopWalking", agent->getAmbulator());
  auto approachBall = make_shared<ApproachBall>("approachBall", agent->getAmbulator());
  auto circleBall = make_shared<CircleBall>("circleBall", agent->getAmbulator(), agent->getHeadModule());
//   auto lookAroundNarrow = make_shared<LookAround>("lookAroundNarrow", agent->getHeadModule(), 45.0);
  auto lookForGoal = make_shared<LookAround>("lookForGoal", agent->getHeadModule(), 100.0, []() { return 1 - 0.33*State::get<CameraFrameState>()->getGoalObservationCount(); });
  auto lookForBall = make_shared<LookAround>("lookForBall", agent->getHeadModule(), 135.0, []() { return State::get<CameraFrameState>()->isBallVisible() ? 0.25 : 1.0; });
  auto lookAtBall = make_shared<LookAtBall>("lookAtBall", agent->getCameraModel(), agent->getHeadModule());
  auto lookAtFeet = make_shared<LookAtFeet>("lookAtFeet", agent->getHeadModule());
  auto lookAtGoal = make_shared<LookAtGoal>("lookAtGoal", agent->getCameraModel(), agent->getHeadModule());

  auto fsm = tree->addOption(make_shared<FSMOption>(agent->getVoice(), "striker"));

  auto standUpState = fsm->newState("standUp", {standUp}, false/*endState*/, true/*startState*/);
  auto lookForBallState = fsm->newState("lookForBall", {stopWalking, lookForBall});
  auto circleToFindLostBallState = fsm->newState("lookForBallCircling", {circleBall});
  auto lookAtBallState = fsm->newState("lookAtBall", {stopWalking, lookAtBall});
  auto approachBallState = fsm->newState("approachBall", {approachBall, lookAtBall});
  auto lookForGoalState = fsm->newState("lookForGoal", {stopWalking, lookForGoal});
  auto lookAtGoalState = fsm->newState("lookAtGoal", {stopWalking, lookAtGoal});
  auto aimState = fsm->newState("aim", {});
  auto circleBallState = fsm->newState("circleBall", {circleBall});
  auto aboutFaceState = fsm->newState("aboutFace", {circleBall});
  auto lookAtFeetState = fsm->newState("lookAtFeet", {lookAtFeet});
  auto leftKickState = fsm->newState("leftKick", {leftKick});
  auto rightKickState = fsm->newState("rightKick", {rightKick});

  setPlayerActivityInStates(agent, PlayerActivity::ApproachingBall, { approachBallState });
  setPlayerActivityInStates(agent, PlayerActivity::Waiting, { standUpState, circleToFindLostBallState, lookForBallState, lookAtBallState });
  setPlayerActivityInStates(agent, PlayerActivity::AttackingGoal, { lookForGoalState, lookAtGoalState, aimState, circleBallState, lookAtFeetState, leftKickState, rightKickState });

  standUpState
    ->transitionTo(lookForBallState, "standing")
    ->whenTerminated();

  lookForBallState
    ->transitionTo(lookAtBallState, "found")
    ->when([]() { return stepUpDownThreshold(5, ballVisibleCondition); });

  // walk a circle if we don't find the ball within some time limit
  lookForBallState
    ->transitionTo(circleToFindLostBallState, "lost-ball-long")
    ->after(chrono::seconds(8));

  // after 5 seconds of circling, look for the ball again
  circleToFindLostBallState
    ->transitionTo(lookForBallState, "done")
    ->after(chrono::seconds(5));

  // stop turning if the ball comes into view
  circleToFindLostBallState
    ->transitionTo(lookAtBallState, "found")
    ->when([]() { return stepUpDownThreshold(5, ballVisibleCondition); });

  lookAtBallState
    ->transitionTo(lookForBallState, "lost-ball")
    ->when(ballLostConditionFactory);

  // start approaching the ball when we have the confidence that it's really there
  // TODO this doesn't filter the ball position, so may be misled by jitter
  lookAtBallState
    ->transitionTo(approachBallState, "found")
    ->when([]() { return stepUpDownThreshold(10, ballVisibleCondition); });

  approachBallState
    ->transitionTo(lookForBallState, "lost-ball")
    ->when(ballLostConditionFactory);

  // stop walking to ball once we're close enough
  approachBallState
    ->transitionTo(lookForGoalState, "near-ball")
    ->when([fsm]()
    {
      // Approach ball until we're within a given distance
      // TODO use filtered ball position
      auto ballObs = State::get<AgentFrameState>()->getBallObservation();
      static auto stoppingDistance = Config::getSetting<double>("options.approach-ball.stop-distance");
      return ballObs && (ballObs->head<2>().norm() < stoppingDistance->getValue());
    });

  // Abort attack if it looks like we are going to kick an own goal
  lookForGoalState
    ->transitionTo(aboutFaceState, "abort-attack-own-goal")
    ->when([]()
    {
      // If the keeper is telling us that the ball is close to our goal, and
      // we see a goalpost nearly that far away, then we should abort the
      // attack.
      auto team = State::get<TeamState>();
      auto agentFrame = State::get<AgentFrameState>();

      auto keeper = team->getKeeperState();
      auto const& goalObses = agentFrame->getGoalObservations();

      auto closestGoalDist = std::numeric_limits<double>::max();
      for (auto const& obs : goalObses)
      {
        auto dist = obs.head<2>().norm();
        if (dist < closestGoalDist)
          closestGoalDist = dist;
      }

      if (!goalObses.empty() && keeper != nullptr && keeper->getAgeMillis() < 10000)
      {
        auto ballObs = keeper->ballRelative;
        if (ballObs.hasValue() && ballObs->norm() < 3 && closestGoalDist < 4)
        {
          log::info("lookForGoalState->aboutFaceState") << "Goalie's ball dist " << ballObs->norm() << ", closest goal dist " << closestGoalDist;
          return true;
        }
      }
      return false;
    });

  lookForGoalState
    ->transitionTo(lookAtGoalState, "see-both-goals")
    ->when([]()
    {
      // TODO use the localiser here rather than requiring both posts to be in frame
      auto goalsObs = State::get<AgentFrameState>()->getGoalObservations();
      return goalsObs.size() >= 2;
    });

  // If we notice the ball is too far to kick, abort kick
  lookForGoalState
    ->transitionTo(lookForBallState, "ball-too-far")
    ->when([]() { return stepUpDownThreshold(6, ballTooFarToKick); });

  // limit how long we will look for the goal
  lookForGoalState
    ->transitionTo(lookAtFeetState, "give-up")
    ->after(chrono::seconds(7));

  aboutFaceState
    ->transitionTo(lookForGoalState, "rotate-done")
    ->after(chrono::seconds(10));

  lookAtGoalState
    ->transitionTo(aimState, "confident")
    ->after(chrono::milliseconds(500));

  // start kick procedure if goal is in front of us
  aimState
    ->transitionTo(lookAtFeetState, "square-to-goal")
    ->when([]()
    {
      double panAngle = State::get<BodyState>(StateTime::CameraImage)->getJoint(JointId::HEAD_PAN)->angleRads;
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
    ->when([circleBallState,agent]()
    {
      // TODO break dependency upon pan limit
      double panAngle = State::get<BodyState>(StateTime::CameraImage)->getJoint(JointId::HEAD_PAN)->angleRads;
      double panAngleRange = agent->getHeadModule()->getLeftLimitRads();
      double panRatio = panAngle / panAngleRange;
      double circleDurationSeconds = fabs(panRatio) * 4.5;
      log::info("circleBallState")
          << "circleDurationSeconds=" << circleDurationSeconds
          << " secondsSinceStart=" << circleBallState->secondsSinceStart()
          << " panRatio=" << panRatio
          << " panAngle=" << panAngle
          << " leftLimitDegs=" << agent->getHeadModule()->getLeftLimitDegs();

      // Return a function that closes over the computed duration
      return [circleBallState,circleDurationSeconds]()
      {
        return circleBallState->secondsSinceStart() >= circleDurationSeconds;
      };
    });

  // If we notice the ball is too far to kick, abort kick
  lookAtFeetState
    ->transitionTo(lookForBallState, "ball-too-far")
    ->when([]() { return stepUpDownThreshold(10, ballTooFarToKick); });

  // TODO if ball too central, step to left/right slightly, or use different kick

  lookAtFeetState
    ->transitionTo(leftKickState, "ball-left")
    ->when([lookAtFeet,lookAtFeetState]()
    {
      // Look at feet for one second
      if (lookAtFeetState->secondsSinceStart() < 1)
        return false;

      // Wait until we've finished looking down
      if (!lookAtFeetState->allOptionsTerminated())
        return false;

      if (lookAtFeet->hasPosition())
      {
        auto ballPos = lookAtFeet->getAverageBallPositionAgentFrame();
        if (ballPos.x() < 0)
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
      // Look at feet for one second
      if (lookAtFeetState->secondsSinceStart() < 1)
        return false;

      // Wait until we've finished looking down
      if (!lookAtFeetState->allOptionsTerminated())
        return false;

      if (lookAtFeet->hasPosition())
      {
        auto ballPos = lookAtFeet->getAverageBallPositionAgentFrame();
        if (ballPos.x() >= 0)
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

  ofstream playingOut("fsm-striker.dot");
  playingOut << fsm->toDot();

  return fsm;
}

///////////////////////////////////////////////////////////////////////////////

shared_ptr<FSMOption> buildPenaltyKeeperFsm(Agent* agent, shared_ptr<OptionTree> tree)
{
  auto standUp = make_shared<MotionScriptOption>("standUpScript", agent->getMotionScriptModule(), "./motionscripts/stand-ready-upright.json");
  auto leftDive = make_shared<MotionScriptOption>("diveLeft", agent->getMotionScriptModule(), "./motionscripts/dive-left.json");
  auto rightDive = make_shared<MotionScriptOption>("diveRight", agent->getMotionScriptModule(), "./motionscripts/dive-right.json");
  auto stopWalking = make_shared<StopWalking>("stopWalking", agent->getAmbulator());
  auto lookAroundNarrow = make_shared<LookAround>("lookAroundNarrow", agent->getHeadModule(), 45.0);
  auto lookAtBall = make_shared<LookAtBall>("lookAtBall", agent->getCameraModel(), agent->getHeadModule());

  auto fsm = tree->addOption(make_shared<FSMOption>(agent->getVoice(), "penalty-keeper"));

  auto standUpState = fsm->newState("standUp", {standUp}, false/*endState*/, true/*startState*/);
  auto lookForBallState = fsm->newState("lookForBall", {stopWalking, lookAroundNarrow});
  auto lookAtBallState = fsm->newState("lookAtBall", {stopWalking, lookAtBall});
  auto leftDiveState = fsm->newState("leftDive", {leftDive});
  auto rightDiveState = fsm->newState("rightDive", {rightDive});

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
        auto ball = State::get<AgentFrameState>()->getBallObservation();
        return ball && ball->y() < 1.0 && ball->x() < -0.1;
      });
    });

  lookAtBallState
    ->transitionTo(rightDiveState, "ball-right")
    ->when([]()
    {
      return trueForMillis(100, []()
      {
        auto ball = State::get<AgentFrameState>()->getBallObservation();
        return ball && ball->y() < 1.0 && ball->x() > 0.1;
      });
    });

  leftDiveState
    ->transitionTo(lookForBallState, "done")
    ->whenTerminated();

  rightDiveState
    ->transitionTo(lookForBallState, "done")
    ->whenTerminated();

  ofstream playingOut("fsm-penalty-keeper.dot");
  playingOut << fsm->toDot();

  return fsm;
}

///////////////////////////////////////////////////////////////////////////////

shared_ptr<FSMOption> buildKeeperFsm(Agent* agent, shared_ptr<OptionTree> tree)
{
  auto standUp = make_shared<MotionScriptOption>("standUpScript", agent->getMotionScriptModule(), "./motionscripts/stand-ready-upright.json");
  auto stopWalking = make_shared<StopWalking>("stopWalking", agent->getAmbulator());
  auto approachBall = make_shared<ApproachBall>("approachBall", agent->getAmbulator());
  auto circleBall = make_shared<CircleBall>("circleBall", agent->getAmbulator(), agent->getHeadModule());
  auto lookAroundNarrow = make_shared<LookAround>("lookAroundNarrow", agent->getHeadModule(), 45.0);
  auto lookForGoal = make_shared<LookAround>("lookForGoal", agent->getHeadModule(), 100.0, []() { return 1 - 0.33*State::get<CameraFrameState>()->getGoalObservationCount(); });
  auto lookForBall = make_shared<LookAround>("lookForBall", agent->getHeadModule(), 135.0, []() { return State::get<CameraFrameState>()->isBallVisible() ? 0.25 : 1.0; });
  auto lookAtBall = make_shared<LookAtBall>("lookAtBall", agent->getCameraModel(), agent->getHeadModule());
  auto lookAtFeet = make_shared<LookAtFeet>("lookAtFeet", agent->getHeadModule());
  auto lookAtGoal = make_shared<LookAtGoal>("lookAtGoal", agent->getCameraModel(), agent->getHeadModule());

  auto leftDive = make_shared<MotionScriptOption>("diveLeft", agent->getMotionScriptModule(), "./motionscripts/dive-left.json");
  auto rightDive = make_shared<MotionScriptOption>("diveRight", agent->getMotionScriptModule(), "./motionscripts/dive-right.json");
  auto bigStepLeft = make_shared<MotionScriptOption>("bigStepLeft", agent->getMotionScriptModule(), "./motionscripts/step-left-big.json");
  auto bigStepRight = make_shared<MotionScriptOption>("bigStepRight", agent->getMotionScriptModule(), "./motionscripts/step-right-big.json");

  auto fsm = tree->addOption(make_shared<FSMOption>(agent->getVoice(), "keeper"));

  // TODO test this further
  // TODO add logic to kick ball away from goal if close to keeper

  auto standUpState = fsm->newState("standUp", {standUp}, false/*endState*/, true/*startState*/);
  auto lookForBallState = fsm->newState("lookForBall", {stopWalking, lookForBall});
  auto lookAtBallState = fsm->newState("lookAtBall", {stopWalking, lookAtBall});
  auto bigStepLeftState = fsm->newState("bigStepLeft", {bigStepLeft});
  auto bigStepRightState = fsm->newState("bigStepRight", {bigStepRight});

  setPlayerActivityInStates(agent, PlayerActivity::Waiting, { standUpState, lookForBallState, lookForBallState, lookAtBallState, bigStepLeftState, bigStepRightState });

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
        auto ball = State::get<AgentFrameState>()->getBallObservation();
        return ball && Range<double>(0.75, 1.5).contains(ball->y()) && Range<double>(-0.75, -0.3).contains(ball->x());
      });
    });

  lookAtBallState
    ->transitionTo(bigStepRightState, "ball-right")
    ->when([]()
    {
      return trueForMillis(1000, []()
      {
        auto ball = State::get<AgentFrameState>()->getBallObservation();
        return ball && Range<double>(0.75, 1.5).contains(ball->y()) && Range<double>(0.3, 0.75).contains(ball->x());
      });
    });

  bigStepLeftState
    ->transitionTo(lookForBallState, "done")
    ->whenTerminated();

  bigStepRightState
    ->transitionTo(lookForBallState, "done")
    ->whenTerminated();

  ofstream playingOut("fsm-keeper.dot");
  playingOut << fsm->toDot();

  return fsm;
}

///////////////////////////////////////////////////////////////////////////////

shared_ptr<OptionTree> AdHocOptionTreeBuilder::buildTree(Agent* agent)
{
  unsigned uniformNumber   = agent->getUniformNumber();
  unsigned teamNumber      = agent->getTeamNumber();
  auto const& ambulator          = agent->getAmbulator();
  auto const& motionScriptModule = agent->getMotionScriptModule();
  auto const& headModule         = agent->getHeadModule();
  auto const& fallDetector       = agent->getFallDetector();

  auto isPenalised = [teamNumber,uniformNumber]()
  {
    auto gameState = State::get<GameState>();
    return gameState && gameState->teamInfo(teamNumber).getPlayer(uniformNumber).hasPenalty();
  };

  auto isNotPenalised = [teamNumber,uniformNumber]()
  {
    auto gameState = State::get<GameState>();
    return gameState && !gameState->teamInfo(teamNumber).getPlayer(uniformNumber).hasPenalty();
  };

  auto nonPenalisedPlayMode = [isNotPenalised](PlayMode playMode)
  {
    return [isNotPenalised,playMode]()
    {
      auto gameState = State::get<GameState>();
      return gameState && isNotPenalised() && gameState->getPlayMode() == playMode;
    };
  };

  auto isWalking = [ambulator]() { return ambulator->isRunning(); };

  auto hasFallenForward = [fallDetector]() { return fallDetector->getFallenState() == FallState::FORWARD; };

  auto hasFallenBackward = [fallDetector]() { return fallDetector->getFallenState() == FallState::BACKWARD; };

  auto isAgentShutdownRequested = changedTo(true, [agent]() { return agent->isStopRequested(); });

  // BUILD TREE

  auto tree = make_shared<OptionTree>();

  // OPTIONS

  auto sit = make_shared<MotionScriptOption>("sitDownScript", motionScriptModule, "./motionscripts/sit-down.json");
  auto sitArmsBack = make_shared<MotionScriptOption>("sitDownScript", motionScriptModule, "./motionscripts/sit-down-arms-back.json");
  auto standUp = make_shared<MotionScriptOption>("standUpScript", motionScriptModule, "./motionscripts/stand-ready-upright.json");
  auto forwardGetUp = make_shared<MotionScriptOption>("forwardGetUpScript", motionScriptModule, "./motionscripts/get-up-from-front.json");
  auto backwardGetUp = make_shared<MotionScriptOption>("backwardGetUpScript", motionScriptModule, "./motionscripts/get-up-from-back.json");
  auto stopWalking = make_shared<StopWalking>("stopWalking", ambulator);

  auto performRole = make_shared<DispatchOption<PlayerRole>>("performRole", [agent](){ return agent->getBehaviourControl()->getPlayerRole(); });
  performRole->setOption(PlayerRole::Keeper, buildKeeperFsm(agent, tree));
  performRole->setOption(PlayerRole::Striker, buildStrikerFsm(agent, tree));
  // NOTE for now we re-use the same striker behaviour for the regular striker as the penalty striker
  auto strikerFsm = buildPenaltyKeeperFsm(agent, tree);
  performRole->setOption(PlayerRole::PenaltyStriker, strikerFsm);
  performRole->setOption(PlayerRole::PenaltyKeeper, strikerFsm);

  //
  // Build the top-level FSM
  //

  auto winFsm = make_shared<FSMOption>(agent->getVoice(), "win");

  // The win FSM is the root of the option tree
  tree->addOption(winFsm, /*isRoot*/ true);

  auto startUpState = winFsm->newState("startUp", {sit}, false/*end state*/, true/* start state */);
  auto readyState = winFsm->newState("ready", {stopWalking});
  auto pausing1State = winFsm->newState("pausing1", {stopWalking});
  auto pausing2State = winFsm->newState("pausing2", {sit});
  auto pausedState = winFsm->newState("paused", {});
  auto unpausingState = winFsm->newState("unpausing", {standUp});
  auto setState = winFsm->newState("set", {stopWalking});
  auto playingState = winFsm->newState("playing", {performRole});
  auto penalizedState = winFsm->newState("penalized", {stopWalking});
  auto forwardGetUpState = winFsm->newState("forwardGetUp", {forwardGetUp});
  auto backwardGetUpState = winFsm->newState("backwardGetUp", {backwardGetUp});
  auto stopWalkingForShutdownState = winFsm->newState("stopWalkingForShutdown", {stopWalking});
  auto sitForShutdownState = winFsm->newState("sitForShutdown", {sitArmsBack});
  auto stopAgentAndExitState = winFsm->newState("stopAgentAndExit", {});

  // In the Win FSM, any state other than 'playing' corresponds to the 'waiting' activity.
  setPlayerActivityInStates(agent,
    PlayerActivity::Waiting,
    { startUpState, readyState, pausing1State, pausing2State, pausedState,
      unpausingState, setState, penalizedState, forwardGetUpState,
      backwardGetUpState, stopWalkingForShutdownState, sitForShutdownState,
      stopAgentAndExitState });

  // In the Win FSM, any state other than 'playing' and 'penalised' corresponds to the 'inactive' status.
  setPlayerStatusInStates(agent,
    PlayerStatus::Inactive,
    { startUpState, readyState, pausing1State, pausing2State, pausedState,
      unpausingState, setState, forwardGetUpState,
      backwardGetUpState, stopWalkingForShutdownState, sitForShutdownState,
      stopAgentAndExitState });
  setPlayerStatusInStates(agent, PlayerStatus::Active, { playingState });
  setPlayerStatusInStates(agent, PlayerStatus::Penalised, { penalizedState });

  auto const& debugger = agent->getDebugger();
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

  ofstream winOut("fsm-win.dot");
  winOut << winFsm->toDot();

  return tree;
}

