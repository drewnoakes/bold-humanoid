#include "adhocoptiontreebuilder.ih"

auto shouldYieldToOtherAttacker = []()
{
  auto team = State::get<TeamState>();
  auto agentFrame = State::get<AgentFrameState>();

  if (!team || !agentFrame || !agentFrame->isBallVisible())
    return false;

  double dist = agentFrame->getBallObservation()->norm();

  bool isTeamMateAttacking = team->isTeamMate(PlayerActivity::AttackingGoal);

  static auto yieldMinDist = Config::getSetting<double>("options.yield.min-dist");
  static auto yieldMaxDist = Config::getSetting<double>("options.yield.max-dist");

  return dist > yieldMinDist->getValue() && dist < yieldMaxDist->getValue() && isTeamMateAttacking;
};

auto ballIsStoppingDistance = []()
{
  // Approach ball until we're within a given distance
  // TODO use filtered ball position
  auto ballObs = State::get<AgentFrameState>()->getBallObservation();
  static auto stoppingDistance = Config::getSetting<double>("options.approach-ball.stop-distance");
  return ballObs && (ballObs->head<2>().norm() < stoppingDistance->getValue());
};

auto isPerfectLineForAttack = []()
{
  static auto isPerfectLineEnabled = Config::getSetting<bool>("options.perfect-line.enabled");

  if (!isPerfectLineEnabled->getValue())
    return false;

  auto agentFrame = State::get<AgentFrameState>();
  if (!agentFrame->isBallVisible())
    return false;

  auto const& goals = agentFrame->getGoalObservations();
  if (goals.size() != 2)
    return false;

  // ASSUME we are looking at the ball
  // Must be looking approximately straight ahead (the ball is directly in front of us)
  double panAngle = State::get<BodyState>(StateTime::CameraImage)->getJoint(JointId::HEAD_PAN)->angleRads;
  if (fabs(Math::radToDeg(panAngle)) > 5.0)
    return false;

  // Verify goals are approximately the correct distance apart
  double goalDist = (goals[0] - goals[1]).norm();
  if (fabs(goalDist - FieldMap::getGoalY()) > FieldMap::getGoalY()/3.0)
    return false;

  // If we have team data...
  auto team = State::get<TeamState>();
  if (team && team->getKeeperState())
  {
    // Verify the distance from the ball to the goal midpoint is sufficiently
    // different to the keeper's observed distance, if they see the ball.
    auto keeperBall = team->getKeeperState()->ballRelative;
    if (team->getKeeperState()->getAgeMillis() < 5000 && keeperBall.hasValue())
    {
      Vector3d goalMidpoint = (goals[0] + goals[1]) / 2.0;
      double ballToGoalDist = (agentFrame->getBallObservation().value() - goalMidpoint).norm();

      if (fabs(keeperBall->norm() - ballToGoalDist) < 1.5)
        return false;
    }
  }

  // The goals must appear to either side of the ball
  double ballX = agentFrame->getBallObservation()->x();
  bool isRight0 = goals[0].x() > ballX;
  bool isRight1 = goals[1].x() > ballX;
  if (isRight0 == isRight1)
    return false;

  // All checks pass - transition to direct attack
  return true;
};

shared_ptr<FSMOption> AdHocOptionTreeBuilder::buildStrikerFsm(Agent* agent, shared_ptr<OptionTree> tree)
{
  auto buildStationaryMap = make_shared<BuildStationaryMap>("buildStationaryMap");
  auto standUp = make_shared<MotionScriptOption>("standUpScript", agent->getMotionScriptModule(), "./motionscripts/stand-ready-upright.json", /* ifNotInFinalPose */ true);
  auto leftKick = make_shared<MotionScriptOption>("leftKickScript", agent->getMotionScriptModule(), "./motionscripts/kick-left.json");
  auto rightKick = make_shared<MotionScriptOption>("rightKickScript", agent->getMotionScriptModule(), "./motionscripts/kick-right.json");
  auto stopWalking = make_shared<StopWalking>("stopWalking", agent->getWalkModule());
  auto approachBall = make_shared<ApproachBall>("approachBall", agent->getWalkModule(), agent->getBehaviourControl());
  auto lookForGoal = make_shared<LookAround>("lookForGoal", agent->getHeadModule(), 100.0, []() { return 1 - 0.33*State::get<CameraFrameState>()->getGoalObservationCount(); });
  auto lookForBall = make_shared<LookAround>("lookForBall", agent->getHeadModule(), 135.0, []() { return State::get<CameraFrameState>()->isBallVisible() ? 0.15 : 0.5; });
  auto lookAtBall = make_shared<LookAtBall>("lookAtBall", agent->getCameraModel(), agent->getHeadModule());
  auto lookAtFeet = make_shared<LookAtFeet>("lookAtFeet", agent->getHeadModule());
  auto circleBall = make_shared<CircleBall>("circleBall", agent);
  auto searchBall = make_shared<SearchBall>("searchBall", agent->getWalkModule(), agent->getHeadModule());
  auto kick = make_shared<KickOption>("kick", agent);

  auto fsm = tree->addOption(make_shared<FSMOption>(agent->getVoice(), "striker"));

  auto standUpState = fsm->newState("standUp", {standUp}, false/*endState*/, true/*startState*/);
  auto lookForBallState = fsm->newState("lookForBall", {stopWalking, lookForBall, buildStationaryMap});
  auto circleToFindLostBallState = fsm->newState("lookForBallCircling", {searchBall});
  auto lookAtBallState = fsm->newState("lookAtBall", {stopWalking, lookAtBall});
  auto approachBallState = fsm->newState("approachBall", {approachBall, lookAtBall});
  auto directAttackState = fsm->newState("directAttack", {approachBall, lookAtBall});
  auto atBallState = fsm->newState("atBall", {stopWalking, lookForGoal, buildStationaryMap});
  auto aimState = fsm->newState("aim", {});
  auto turnToGoalState = fsm->newState("turnToGoal", {circleBall});
  auto aboutFaceState = fsm->newState("aboutFace", {circleBall});
  auto lookAtFeetState = fsm->newState("lookAtFeet", {lookAtFeet,stopWalking});
  auto leftKickState = fsm->newState("leftKick", {leftKick});
  auto rightKickState = fsm->newState("rightKick", {rightKick});
  auto kickState = fsm->newState("kick", {kick});
  auto waitForOtherStrikerState = fsm->newState("wait", {stopWalking,lookAtBall});

  // NOTE we set either ApproachingBall or AttackingGoal in approachBall option directly
//  setPlayerActivityInStates(agent, PlayerActivity::ApproachingBall, { approachBallState });
  setPlayerActivityInStates(agent, PlayerActivity::Waiting, { standUpState, circleToFindLostBallState, lookForBallState, lookAtBallState, waitForOtherStrikerState });
  setPlayerActivityInStates(agent, PlayerActivity::AttackingGoal, { atBallState, aimState, turnToGoalState, lookAtFeetState, leftKickState, rightKickState });

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
  lookAtBallState
    ->transitionTo(approachBallState, "confident")
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
    ->transitionTo(atBallState, "near-ball")
    ->when(ballIsStoppingDistance);

  approachBallState
    ->transitionTo(directAttackState, "good-line")
    ->when(isPerfectLineForAttack);

  directAttackState
    ->transitionTo(kickForwardsState, "near-ball")
    ->when(ballIsStoppingDistance);

  waitForOtherStrikerState
    ->transitionTo(lookAtBallState)
    ->when([]() { return stepUpDownThreshold(10, negate(shouldYieldToOtherAttacker)); });

  auto maxGoalieGoalDistance      = Config::getSetting<double>("vision.player-detection.max-goalie-goal-dist");

  // Abort attack if it looks like we are going to kick an own goal
  atBallState
    ->transitionTo(aboutFaceState, "abort-attack-own-goal")
    ->when([agent,maxGoalieGoalDistance]()
    {
      // If the keeper is telling us that the ball is close to our goal, and
      // we see a goalpost nearly that far away, then we should abort the
      // attack.
      auto team = State::get<TeamState>();
      auto agentFrame = State::get<AgentFrameState>();

      if (team == nullptr || agentFrame == nullptr)
        return false;

      FieldSide ballSide = team->getKeeperBallSideEstimate();

      if (ballSide == FieldSide::Unknown)
      {
        // Check if we see goalie in between goal posts
        auto goalObservations = agentFrame->getGoalObservations();
        auto teamMateObservations = agentFrame->getTeamMateObservations();
        if (goalObservations.size() == 2 && teamMateObservations.size() > 0)
        {
          auto goalMidpointAgentFrame = (goalObservations[0] + goalObservations[1]) / 2.0;
          for (auto const& teamMateObs : teamMateObservations)
            if ((goalMidpointAgentFrame - teamMateObs).head<2>().norm() < maxGoalieGoalDistance->getValue())
              return true;
        }
        return false;
      }

      auto closestGoalObs = agentFrame->getClosestGoalObservation();

      if (!closestGoalObs.hasValue())
        return false;

      // ASSUME the ball is approximately at our feet

      double closestGoalDist = closestGoalObs->norm();

      const double maxPositionMeasurementError = 0.4; // TODO review this experimentally

      if (ballSide == FieldSide::Ours && closestGoalDist < (FieldMap::fieldLengthX()/2.0) - maxPositionMeasurementError)
      {
        log::info("lookForGoalState->aboutFaceState") << "Keeper believes ball is on our side, and closest goal is too close at " << closestGoalDist;
        return true;
      }

      double theirsThreshold = Vector2d(
        (FieldMap::fieldLengthX() / 2.0) + maxPositionMeasurementError,
         FieldMap::fieldLengthY() / 2.0
      ).norm();

      if (ballSide == FieldSide::Theirs && closestGoalDist > theirsThreshold)
      {
        log::info("lookForGoalState->aboutFaceState") << "Keeper believes ball is on the opponent's side, and closest goal is too far at " << closestGoalDist;
        return true;
      }

      return false;
    });

  // If we notice the ball is too far to kick, abort kick
  atBallState
    ->transitionTo(lookForBallState, "ball-too-far")
    ->when([]() { return stepUpDownThreshold(6, ballTooFarToKick); });

  atBallState
    ->transitionTo(kickState, "can-kick")
    ->when([kick]() { return kick->canKick(); });

  kickState
    ->transitionTo(lookForBallState, "done")
    ->whenTerminated();

  // limit how long we will look for the goal
  atBallState
    ->transitionTo(kickForwardsState, "give-up")
    ->after(chrono::seconds(7));

  aboutFaceState
    ->transitionTo(atBallState, "rotate-done")
    ->after(chrono::seconds(10));

  // start kick procedure if goal is in front of us
  aimState
    ->transitionTo(kickForwardsState, "square-to-goal")
    ->when([]()
    {
      double panAngle = State::get<BodyState>(StateTime::CameraImage)->getJoint(JointId::HEAD_PAN)->angleRads;
      // TODO this angular limit in config
      return fabs(Math::radToDeg(panAngle)) < 25.0;
    });

  // circle immediately, if goal is not in front (prior transition didn't fire)
  // TODO consider static map when turning to goal -- don't require us seeing both posts at the same time
  //      - test this by putting a bot in front of the goal where it cannot see both goal posts, facing sideways across the field -- it should still turn to the goal instead of doing a give-up kick
  aimState
    ->transitionTo(turnToGoalState, "goal-at-angle")
    ->when([]() { return true; });

  // control duration of ball circling
  turnToGoalState
    ->transitionTo(atBallState, "done")
    ->when([circleBall,turnToGoalState,agent]()
    {
      // TODO break dependency upon pan limit
      double panAngle = State::get<BodyState>(StateTime::CameraImage)->getJoint(JointId::HEAD_PAN)->angleRads;
      double panAngleRange = agent->getHeadModule()->getLeftLimitRads();
      double panRatio = panAngle / panAngleRange;
      double circleDurationSeconds = fabs(panRatio) * Config::getValue<double>("options.turn-to-goal.time-scaling");
      log::info("circleBallState")
          << "circleDurationSeconds=" << circleDurationSeconds
          << " secondsSinceStart=" << turnToGoalState->secondsSinceStart()
          << " panRatio=" << panRatio
          << " panAngle=" << panAngle
          << " leftLimitDegs=" << agent->getHeadModule()->getLeftLimitDegs();

      // TODO assigning state in this condition factory is not very explicit or clear
      circleBall->setIsLeftTurn(panAngle < 0);

      // Return a function that closes over the computed duration
      return [turnToGoalState,circleDurationSeconds]()
      {
        return turnToGoalState->secondsSinceStart() >= circleDurationSeconds;
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
