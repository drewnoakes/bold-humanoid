#include "adhocoptiontreebuilder.hh"

#include "../../Option/AwaitTheirKickOff/awaittheirkickoff.hh"
#include "../../Option/ApproachBall/approachball.hh"
#include "../../Option/AtBall/atball.hh"
#include "../../Option/CircleBall/circleball.hh"
#include "../../Option/LookAround/lookaround.hh"
#include "../../Option/LookAtFeet/lookatfeet.hh"
#include "../../Option/LocateBall/locateball.hh"
#include "../../Option/MotionScriptOption/motionscriptoption.hh"
#include "../../Option/SearchBall/searchball.hh"
#include "../../Option/SequenceOption/sequenceoption.hh"
#include "../../Option/StopWalking/stopwalking.hh"
#include "../../Option/Support/support.hh"
#include "../../State/state.hh"
#include "../../StateObject/BodyState/bodystate.hh"
#include "../../StateObject/GameState/gamestate.hh"
#include "../../StateObject/StationaryMapState/stationarymapstate.hh"
#include "conditionals.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

auto shouldYieldToOtherAttacker = []
{
  auto team = State::get<TeamState>();
  auto agentFrame = State::get<AgentFrameState>();

  if (!team || !agentFrame || !agentFrame->isBallVisible())
    return false;

  bool isTeamMateAttacking = team->isTeamMateInActivity(PlayerActivity::AttackingGoal);
  if (isTeamMateAttacking)
    return true;

  double ballDist = agentFrame->getBallObservation()->norm();
  auto ballObservers = team->getBallObservers();
  bool isTeamMateApproachingBallAndNearer = false;
  for (auto player : ballObservers)
  {
    if (player.activity == PlayerActivity::ApproachingBall && player.ballRelative->norm() < ballDist)
    {
      isTeamMateApproachingBallAndNearer = true;
      break;
    }
  }
  return isTeamMateApproachingBallAndNearer;
};

auto isWithinTenSecondsOfTheirKickOff = []
{
  auto game = State::get<GameState>();
  return game && game->isWithinTenSecondsOfKickOff(Team::Them);
};

auto isWithinTenSecondsOfOurKickOff = []
{
  auto game = State::get<GameState>();
  return game && game->isWithinTenSecondsOfKickOff(Team::Us);
};

// Indicates whether at this moment we observe the ball straight in front of us,
// and in line with our goal such that we should walk directly to it, and kick
// straight without looking around once we reach the ball.
// Unlike the building of the stationary map, this is based on a single cycle's
// observation and so may be used during motion (when not stationary.)
auto isPerfectLineForAttack = []
{
  static auto isPerfectLineEnabled = Config::getSetting<bool>("options.perfect-line.enabled");

  if (!isPerfectLineEnabled->getValue())
    return false;

  auto agentFrame = State::get<AgentFrameState>();
  if (!agentFrame->isBallVisible())
    return false;

  auto goals = agentFrame->getGoalObservations();
  if (goals.size() != 2)
    return false;

  // ASSUME we are looking at the ball
  // Must be looking approximately straight ahead (the ball is directly in front of us)
  double panAngle = State::get<BodyState>(StateTime::CameraImage)->getJoint(JointId::HEAD_PAN)->getAngleRads();
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
    GoalLabel label = GoalLabel::Unknown;

    auto post1Pos = Average<Eigen::Vector2d>{};
    post1Pos.add(goals[0].head<2>());
    auto post2Pos = Average<Eigen::Vector2d>{};
    post2Pos.add(goals[1].head<2>());

    auto keeper = team->getKeeperState();
      
    if (keeper && keeper->ballRelative.hasValue())
    {
      auto agentBallPos = agentFrame->getBallObservation();
      label = StationaryMapState::labelGoalByKeeperBallPosition(
        post1Pos,
        post2Pos,
        *keeper->ballRelative,
        agentBallPos.hasValue() ? Vector2d{agentBallPos->head<2>()} : Vector2d::Zero());
    }
      
    if (label == GoalLabel::Unknown)
      label = StationaryMapState::labelGoalByKeeperBallDistance(post1Pos, post2Pos, team->getKeeperBallSideEstimate());
    
    if (label == GoalLabel::Ours)
      return false;
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

shared_ptr<FSMOption> AdHocOptionTreeBuilder::buildStrikerFsm(Agent* agent)
{
  // OPTIONS

  auto standUp = make_shared<MotionScriptOption>("stand-up-script", agent->getMotionScriptModule(), "./motionscripts/stand-ready-upright.json", /*ifNotInFinalPose*/true);
  auto leftKick = make_shared<MotionScriptOption>("left-kick-script", agent->getMotionScriptModule(), "./motionscripts/kick-left.json");
  auto rightKick = make_shared<MotionScriptOption>("right-kick-script", agent->getMotionScriptModule(), "./motionscripts/kick-right.json");
  auto stopWalking = make_shared<StopWalking>("stop-walking", agent->getWalkModule());
  auto locateBall = make_shared<LocateBall>("locate-ball", agent, 135.0, LookAround::speedIfBallVisible(0.15));
  auto approachBall = make_shared<ApproachBall>("approach-ball", agent->getWalkModule(), agent->getBehaviourControl());
  auto kickMotion = make_shared<MotionScriptOption>("kick", agent->getMotionScriptModule());
  auto atBall = make_shared<AtBall>("at-ball", agent);
  auto lookAtBall = make_shared<LookAtBall>("look-at-ball", agent->getCameraModel(), agent->getHeadModule());
  auto lookAtFeet = make_shared<LookAtFeet>("look-at-feet", agent->getHeadModule());
  auto circleBall = make_shared<CircleBall>("circle-ball", agent);
  auto searchBall = make_shared<SearchBall>("search-ball", agent->getWalkModule(), agent->getHeadModule());
//  auto awaitTheirKickOff = make_shared<AwaitTheirKickOff>("await-their-kick-off");
  auto support = make_shared<Support>("support", agent->getWalkModule());

  // STATES

  auto fsm = make_shared<FSMOption>(agent->getVoice(), "striker");

  auto standUpState = fsm->newState("stand-up", { standUp }, /*endState*/false, /*startState*/true);
  auto locateBallState = fsm->newState("locate-ball", { stopWalking, locateBall });
  auto locateBallCirclingState = fsm->newState("locate-ball-circling", { searchBall });
  auto approachBallState = fsm->newState("approach-ball", { approachBall, lookAtBall });
  auto atBallState = fsm->newState("at-ball", { stopWalking, atBall });
  auto turnAroundBallState = fsm->newState("turn-around-ball", { circleBall });
  auto kickForwardsState = fsm->newState("kick-forwards", { stopWalking, lookAtFeet });
  auto leftKickState = fsm->newState("left-kick", { leftKick });
  auto rightKickState = fsm->newState("right-kick", { rightKick });
  auto kickState = fsm->newState("kick", { SequenceOption::make("stop-walking-and-kick-sequence", { stopWalking, kickMotion }) });
  auto yieldState = fsm->newState("yield", { stopWalking, lookAtBall });
//  auto awaitTheirKickOffState = fsm->newState("await-their-kick-off", { stopWalking, locateBall, awaitTheirKickOff });
  auto supportState = fsm->newState("support", { support, lookAtBall });

  // NOTE we set either ApproachingBall or AttackingGoal in approachBall option directly
  //  setPlayerActivityInStates(agent, PlayerActivity::ApproachingBall, { approachBallState });
  setPlayerActivityInStates(agent, PlayerActivity::Waiting, { standUpState/*, awaitTheirKickOffState*/, locateBallCirclingState, locateBallState, yieldState, supportState });

  setPlayerActivityInStates(agent, PlayerActivity::AttackingGoal, { atBallState, turnAroundBallState, kickForwardsState, leftKickState, rightKickState });

  // TRANSITIONS

//  standUpState
//    ->transitionTo(awaitTheirKickOffState)
//    ->when(isWithinTenSecondsOfTheirKickOff);

  standUpState
    ->transitionTo(locateBallState, "standing")
    ->whenTerminated();

//  awaitTheirKickOffState
//    ->transitionTo(locateBallState)
//    ->whenTerminated();

  // start approaching the ball when we have the confidence that it's really there
  locateBallState
    ->transitionTo(approachBallState, "found-ball")
    ->when([] { return stepUpDownThreshold(10, ballVisibleCondition); });

  // walk a circle if we don't find the ball within some time limit
  locateBallState
    ->transitionTo(locateBallCirclingState, "lost-ball-long")
    ->after(chrono::seconds(12));

  // after 10 seconds of circling, look for the ball again
  locateBallCirclingState
    ->transitionTo(locateBallState, "done")
    ->after(chrono::seconds(10));

  // stop turning if the ball comes into view
  locateBallCirclingState
    ->transitionTo(locateBallState, "found-ball")
    ->when([] { return stepUpDownThreshold(5, ballVisibleCondition); });

  approachBallState
    ->transitionTo(locateBallState, "lost-ball")
    ->when(ballLostConditionFactory);

  approachBallState
    ->transitionTo(kickForwardsState, "kick-off")
    ->when([] { return ballIsStoppingDistance() && isWithinTenSecondsOfOurKickOff(); });

  // Let another player shine if they're closer and attempting to score
  approachBallState
    ->transitionTo(supportState, "support")
    ->when([] { return stepUpDownThreshold(10, shouldYieldToOtherAttacker); });

  // stop walking to ball once we're close enough
  approachBallState
    ->transitionTo(atBallState, "near-ball")
    ->when(ballIsStoppingDistance);

  yieldState
    ->transitionTo(locateBallState, "resume")
    ->when([] { return stepUpDownThreshold(10, negate(shouldYieldToOtherAttacker)); });

  supportState
    ->transitionTo(locateBallState, "resume")
    ->when([] { return stepUpDownThreshold(10, negate(shouldYieldToOtherAttacker)); });

  //
  // AT-BALL EXIT TRANSITIONS
  //

  atBallState
    ->transitionTo(kickState, "can-kick")
    ->when([kickMotion]
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
    ->when([circleBall,kickMotion]
    {
      auto map = State::get<StationaryMapState>();
      if (!map)
        return false;
      double turnAngle = map->getTurnAngleRads();
      if (turnAngle == 0)
        return false;
      // TODO provide onResetBefore virtual on Option, and capture this there
      circleBall->setTurnParams(turnAngle, map->getTurnBallPos());

      auto kick = map->getTurnForKick();
      ASSERT(kick);
      kickMotion->setMotionScript(kick->getMotionScript());

      return true;
    });

  // limit how long we will look for the goal
  atBallState
    ->transitionTo(kickForwardsState, "give-up")
    ->after(chrono::seconds(7));

  // If we notice the ball is too far to kick, abort kick
  atBallState
    ->transitionTo(locateBallState, "ball-too-far")
    ->when([] { return stepUpDownThreshold(6, ballTooFarToKick); });

  kickState
    ->transitionTo(locateBallState, "done")
    ->whenTerminated();

  turnAroundBallState
    ->transitionTo(kickState, "done")
//    ->transitionTo(atBallState, "done")
    ->whenTerminated();

  turnAroundBallState
    ->transitionTo(locateBallState, "ball-too-far")
    ->when([] { return stepUpDownThreshold(10, ballTooFarToKick); });

  turnAroundBallState
    ->transitionTo(locateBallState, "lost-ball")
    ->when(ballLostConditionFactory);

  //
  // KICK FORWARDS
  //

  // If we notice the ball is too far to kick, abort kick
  kickForwardsState
    ->transitionTo(locateBallState, "ball-too-far")
    ->when([] { return stepUpDownThreshold(10, ballTooFarToKick); });

  // TODO if ball too central, step to left/right slightly, or use different kick

  kickForwardsState
    ->transitionTo(leftKickState, "ball-left")
    ->when([lookAtFeet,kickForwardsState]
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
          log::info("kickForwardsState->leftKickState") << "Kicking with left foot when ball at (" << ballPos.x() << "," << ballPos.y() << ")";
          return true;
        }
      }
      return false;
    });

  kickForwardsState
    ->transitionTo(rightKickState, "ball-right")
    ->when([lookAtFeet,kickForwardsState]
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
          log::info("kickForwardsState->rightKickState") << "Kicking with right foot when ball at (" << ballPos.x() << "," << ballPos.y() << ")";
          return true;
        }
      }
      return false;
    });

  kickForwardsState
    ->transitionTo(locateBallState, "ball-gone")
    ->when([kickForwardsState]
    {
      // TODO create and use 'all' operator
      if (kickForwardsState->secondsSinceStart() < 1)
        return false;

      // Wait until we've finished looking down
      return kickForwardsState->allOptionsTerminated();
    });

  leftKickState
    ->transitionTo(locateBallState, "done")
    ->whenTerminated();

  rightKickState
    ->transitionTo(locateBallState, "done")
    ->whenTerminated();

  return fsm;
}
