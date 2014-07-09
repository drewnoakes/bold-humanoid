#include "approachball.hh"

#include "../../BehaviourControl/behaviourcontrol.hh"
#include "../../Math/math.hh"
#include "../../MotionModule/WalkModule/walkmodule.hh"
#include "../../State/state.hh"
#include "../../StateObject/AgentFrameState/agentframestate.hh"

#include <Eigen/Core>

using namespace bold;
using namespace Eigen;
using namespace rapidjson;
using namespace std;

vector<shared_ptr<Option>> ApproachBall::runPolicy(Writer<StringBuffer>& writer)
{
  ASSERT(d_upperTurnLimitDegs->getValue() > d_lowerTurnLimitDegs->getValue());
  ASSERT(d_brakeDistance->getValue() != 0);

  auto agentFrame = State::get<AgentFrameState>();

  if (!agentFrame->isBallVisible())
  {
    writer.String("ballPos").Null();
    return {};
  }

  Vector2d ballPos(agentFrame->getBallObservation()->head<2>());

  writer.String("ballPos").StartArray().Double(ballPos.x()).Double(ballPos.y()).EndArray(2);

  double dist = ballPos.norm();

  // If we're close to the ball, tell team mates that we're attacking
  d_behaviourControl->setPlayerActivity(
    dist < 0.5
      ? PlayerActivity::AttackingGoal
      : PlayerActivity::ApproachingBall);

  // Subtract the stopping distance here so that the bot doesn't stop too suddenly
  if (d_useCustomStopDistance)
  {
    dist -= d_stopDistance;
  }
  else
  {
    static auto stoppingDistance = Config::getSetting<double>("options.approach-ball.stop-distance");
    dist -= stoppingDistance->getValue();
  }

  if (dist < 0)
    dist = 0;

  writer.String("ballDist").Double(dist);

  double speedScaleDueToDistance = Math::clamp(dist/d_brakeDistance->getValue(), 0.0, 1.0);

  writer.String("distSpeed").Double(speedScaleDueToDistance);

  Vector2d target(ballPos + Vector2d(0.04, 0));
  double ballAngleRads = Math::angleToPoint(target);

  double speedScaleDueToAngle = Math::lerp(fabs(ballAngleRads),
                                           Math::degToRad(d_lowerTurnLimitDegs->getValue()),
                                           Math::degToRad(d_upperTurnLimitDegs->getValue()),
                                           1.0,
                                           0.0);

  writer.String("angleSpeed").Double(speedScaleDueToAngle);

  double xSpeed = Math::lerp(speedScaleDueToDistance * speedScaleDueToAngle,
                             d_minForwardSpeed->getValue(),
                             d_maxForwardSpeed->getValue());

  // unspecified units
  double turnSpeed = ballAngleRads * d_turnScale->getValue();

  d_walkModule->setMoveDir(xSpeed, 0);
  d_walkModule->setTurnAngle(turnSpeed);

  writer.String("moveDir").StartArray().Double(xSpeed).Double(0).EndArray(2);

  writer.String("turn").Double(turnSpeed);

  return {};
}
