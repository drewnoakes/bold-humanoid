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

// Indicates whether at this moment we observe the ball straight in front of us,
// and in line with our goal such that we should walk directly to it, and kick
// straight without looking around once we reach the ball.
// Unlike the building of the stationary map, this is based on a single cycle's
// observation and so may be used during motion (when not staionary.)
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
  auto kickMotion = make_shared<MotionScriptOption>("kick", agent->getMotionScriptModule());
  auto atBall = make_shared<AtBall>("atBall", agent);
  auto lookForBall = make_shared<LookAround>("lookForBall", agent->getHeadModule(), 135.0, []() { return State::get<CameraFrameState>()->isBallVisible() ? 0.15 : 0.5; });
  auto lookAtBall = make_shared<LookAtBall>("lookAtBall", agent->getCameraModel(), agent->getHeadModule());
  auto lookAtFeet = make_shared<LookAtFeet>("lookAtFeet", agent->getHeadModule());
  auto circleBall = make_shared<CircleBall>("circleBall", agent);
  auto searchBall = make_shared<SearchBall>("searchBall", agent->getWalkModule(), agent->getHeadModule());

  auto fsm = tree->addOption(make_shared<FSMOption>(agent->getVoice(), "striker"));

  auto standUpState = fsm->newState("standUp", {standUp}, false/*endState*/, true/*startState*/);
  auto lookForBallState = fsm->newState("lookForBall", {stopWalking, buildStationaryMap, lookForBall});
  auto circleToFindLostBallState = fsm->newState("lookForBallCircling", {searchBall});
  auto lookAtBallState = fsm->newState("lookAtBall", {stopWalking, lookAtBall});
  auto approachBallState = fsm->newState("approachBall", {approachBall, lookAtBall});
  auto directAttackState = fsm->newState("directAttack", {approachBall, lookAtBall});
  auto atBallState = fsm->newState("atBall", {stopWalking, buildStationaryMap, atBall});
  auto turnAroundBallState = fsm->newState("turnAroundBall", {circleBall});
  auto kickForwardsState = fsm->newState("kickForwards", {stopWalking,lookAtFeet});
  auto leftKickState = fsm->newState("leftKick", {leftKick});
  auto rightKickState = fsm->newState("rightKick", {rightKick});
  auto kickState = fsm->newState("kick", {kickMotion});
  auto waitForOtherStrikerState = fsm->newState("wait", {stopWalking,lookAtBall});

  // NOTE we set either ApproachingBall or AttackingGoal in approachBall option directly
//  setPlayerActivityInStates(agent, PlayerActivity::ApproachingBall, { approachBallState });
  setPlayerActivityInStates(agent, PlayerActivity::Waiting, { standUpState, circleToFindLostBallState, lookForBallState, lookAtBallState, waitForOtherStrikerState });
  setPlayerActivityInStates(agent, PlayerActivity::AttackingGoal, { atBallState, turnAroundBallState, kickForwardsState, leftKickState, rightKickState });

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

  //
  // AT-BALL EXIT TRANSITIONS
  //

  atBallState
    ->transitionTo(kickState, "can-kick")
    ->when([kickMotion,agent]()
    {
      auto map = State::get<StationaryMapState>();
      if (map && map->canKick())
      {
        auto kick = map->getSelectedKick();
        if (kick != nullptr)
        {
          kickMotion->setMotionScript(kick->getMotionScript());
          return true;
        }
      }
      return false;
    });

  atBallState
    ->transitionTo(turnAroundBallState)
    ->when([circleBall]()
    {
      auto map = State::get<StationaryMapState>();
      if (!map)
        return false;
      double turnAngle = map->getTurnAngleRads();
      if (turnAngle == 0)
        return false;
      // TODO provide onResetBefore virtual on Option, and capure this there
      // TODO copy required agent-frame ball pos to circlBall too
      circleBall->setTurnAngle(turnAngle);
      return true;
    });

  // limit how long we will look for the goal
  atBallState
    ->transitionTo(kickForwardsState, "give-up")
    ->after(chrono::seconds(7));

  // If we notice the ball is too far to kick, abort kick
  atBallState
    ->transitionTo(lookForBallState, "ball-too-far")
    ->when([]() { return stepUpDownThreshold(6, ballTooFarToKick); });

  kickState
    ->transitionTo(lookForBallState, "done")
    ->whenTerminated();

  turnAroundBallState
    ->transitionTo(atBallState, "done")
    ->whenTerminated();

  //
  // KICK FORWARDS
  //

  // If we notice the ball is too far to kick, abort kick
  kickForwardsState
    ->transitionTo(lookForBallState, "ball-too-far")
    ->when([]() { return stepUpDownThreshold(10, ballTooFarToKick); });

  // TODO if ball too central, step to left/right slightly, or use different kick

  kickForwardsState
    ->transitionTo(leftKickState, "ball-left")
    ->when([lookAtFeet,kickForwardsState]()
    {
      // Look at feet for one second
      if (kickForwardsState->secondsSinceStart() < 1)
        return false;

      // Wait until we've finished looking down
      if (!kickForwardsState->allOptionsTerminated())
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

  kickForwardsState
    ->transitionTo(rightKickState, "ball-right")
    ->when([lookAtFeet,kickForwardsState]()
    {
      // Look at feet for one second
      if (kickForwardsState->secondsSinceStart() < 1)
        return false;

      // Wait until we've finished looking down
      if (!kickForwardsState->allOptionsTerminated())
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

  kickForwardsState
    ->transitionTo(lookForBallState, "ball-gone")
    ->when([kickForwardsState]()
    {
      // TODO create and use 'all' operator
      if (kickForwardsState->secondsSinceStart() < 1)
        return false;

      // Wait until we've finished looking down
      return kickForwardsState->allOptionsTerminated();
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
