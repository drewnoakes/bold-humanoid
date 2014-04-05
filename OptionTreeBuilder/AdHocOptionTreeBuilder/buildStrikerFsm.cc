#include "adhocoptiontreebuilder.ih"

auto shouldYieldToOtherAttacker = []()
{
  auto team = State::get<TeamState>();
  auto agentFrame = State::get<AgentFrameState>();

  if (!team || !agentFrame || !agentFrame->isBallVisible())
    return false;

  double dist = agentFrame->getBallObservation()->norm();

  bool isTeamMateAttacking = team->isTeamMate(PlayerActivity::AttackingGoal);

  cout << "YIELD isTeamMateAttacking=" << isTeamMateAttacking << " dist=" << dist << endl;

  if (dist > 0.5 && dist < 1.5 && isTeamMateAttacking)
    return true;

  return false;
};

shared_ptr<FSMOption> AdHocOptionTreeBuilder::buildStrikerFsm(Agent* agent, shared_ptr<OptionTree> tree)
{
  auto standUp = make_shared<MotionScriptOption>("standUpScript", agent->getMotionScriptModule(), "./motionscripts/stand-ready-upright.json");
  auto leftKick = make_shared<MotionScriptOption>("leftKickScript", agent->getMotionScriptModule(), "./motionscripts/kick-left.json");
  auto rightKick = make_shared<MotionScriptOption>("rightKickScript", agent->getMotionScriptModule(), "./motionscripts/kick-right.json");
  auto stopWalking = make_shared<StopWalking>("stopWalking", agent->getAmbulator());
  auto approachBall = make_shared<ApproachBall>("approachBall", agent->getAmbulator());
//   auto lookAroundNarrow = make_shared<LookAround>("lookAroundNarrow", agent->getHeadModule(), 45.0);
  auto lookForGoal = make_shared<LookAround>("lookForGoal", agent->getHeadModule(), 100.0, []() { return 1 - 0.33*State::get<CameraFrameState>()->getGoalObservationCount(); });
  auto lookForBall = make_shared<LookAround>("lookForBall", agent->getHeadModule(), 135.0, []() { return State::get<CameraFrameState>()->isBallVisible() ? 0.15 : 1.0; });
  auto lookAtBall = make_shared<LookAtBall>("lookAtBall", agent->getCameraModel(), agent->getHeadModule());
  auto lookAtFeet = make_shared<LookAtFeet>("lookAtFeet", agent->getHeadModule());
  auto lookAtGoal = make_shared<LookAtGoal>("lookAtGoal", agent->getCameraModel(), agent->getHeadModule());
  auto circleBall = make_shared<CircleBall>("circleBall", agent->getAmbulator(), agent->getHeadModule(), lookAtFeet, lookAtBall);
  auto searchBall = make_shared<SearchBall>("searchBall", agent->getAmbulator(), agent->getHeadModule());

  auto fsm = tree->addOption(make_shared<FSMOption>(agent->getVoice(), "striker"));

  auto standUpState = fsm->newState("standUp", {standUp}, false/*endState*/, true/*startState*/);
  auto lookForBallState = fsm->newState("lookForBall", {stopWalking, lookForBall});
  auto circleToFindLostBallState = fsm->newState("lookForBallCircling", {searchBall});
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
  auto waitForOtherStrikerState = fsm->newState("wait", {stopWalking,lookAtBall});

  setPlayerActivityInStates(agent, PlayerActivity::ApproachingBall, { approachBallState });
  setPlayerActivityInStates(agent, PlayerActivity::Waiting, { standUpState, circleToFindLostBallState, lookForBallState, lookAtBallState, waitForOtherStrikerState });
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

  // after 10 seconds of circling, look for the ball again
  circleToFindLostBallState
    ->transitionTo(lookForBallState, "done")
    ->after(chrono::seconds(10));

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

  // Let another player shine if they're closer and attempting to score
  approachBallState
    ->transitionTo(waitForOtherStrikerState, "yield")
    ->when([]() { return stepUpDownThreshold(10, shouldYieldToOtherAttacker); });

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

  waitForOtherStrikerState
    ->transitionTo(lookAtBallState)
    ->after(chrono::seconds(5));

  waitForOtherStrikerState
    ->transitionTo(lookAtBallState)
    ->when([]() { return stepUpDownThreshold(10, negate(shouldYieldToOtherAttacker)); });

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
    ->transitionTo(lookForGoalState, "done")
    ->when([circleBall,circleBallState,agent]()
    {
      // TODO break dependency upon pan limit
      double panAngle = State::get<BodyState>(StateTime::CameraImage)->getJoint(JointId::HEAD_PAN)->angleRads;
      double panAngleRange = agent->getHeadModule()->getLeftLimitRads();
      double panRatio = panAngle / panAngleRange;
      double circleDurationSeconds = fabs(panRatio) * 10;
      log::info("circleBallState")
          << "circleDurationSeconds=" << circleDurationSeconds
          << " secondsSinceStart=" << circleBallState->secondsSinceStart()
          << " panRatio=" << panRatio
          << " panAngle=" << panAngle
          << " leftLimitDegs=" << agent->getHeadModule()->getLeftLimitDegs();

      // TODO assigning state in this condition factory is not very explicit or clear
      circleBall->setIsLeftTurn(panAngle < 0);

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
