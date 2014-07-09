#include "approachball.hh"

#include "../../BehaviourControl/behaviourcontrol.hh"
#include "../../Drawing/drawing.hh"
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

  double ballDist = ballPos.norm();

  // If we're close to the ball, tell team mates that we're attacking
  d_behaviourControl->setPlayerActivity(
    ballDist < 0.5
      ? PlayerActivity::AttackingGoal
      : PlayerActivity::ApproachingBall);

  static auto stoppingDistance = Config::getSetting<double>("options.approach-ball.stop-distance");

  // Subtract the stopping distance here so that the bot doesn't stop too suddenly
  double walkDist = d_useCustomStopDistance
    ? ballDist - d_stopDistance
    : ballDist - stoppingDistance->getValue();

  if (walkDist < 0)
    walkDist = 0;

  writer.String("ballDist").Double(ballDist);
  writer.String("walkDist").Double(walkDist);

  double speedScaleDueToDistance = Math::clamp(walkDist/d_brakeDistance->getValue(), 0.0, 1.0);

  writer.String("distSpeed").Double(speedScaleDueToDistance);

  Vector2d target(ballPos + Vector2d(0.04, 0));
  double targetAngleRads = Math::angleToPoint(target);

  double speedScaleDueToAngle = Math::lerp(fabs(targetAngleRads),
                                           Math::degToRad(d_lowerTurnLimitDegs->getValue()),
                                           Math::degToRad(d_upperTurnLimitDegs->getValue()),
                                           1.0,
                                           0.0);

  writer.String("angleSpeed").Double(speedScaleDueToAngle);

  double xSpeed = Math::lerp(speedScaleDueToDistance * speedScaleDueToAngle,
                             d_minForwardSpeed->getValue(),
                             d_maxForwardSpeed->getValue());

  // unspecified units
  double turnSpeed = targetAngleRads * d_turnScale->getValue();

  double ySpeed = 0;

  // try to avoid any obstacles
  static auto avoidObstacles = Config::getSetting<bool>("options.approach-ball.avoid-obstacles.enabled");
  static auto laneWidth = Config::getSetting<double>("options.approach-ball.avoid-obstacles.lane-width");
  static auto avoidSpeed = Config::getSetting<double>("options.approach-ball.avoid-obstacles.avoid-speed");
  if (avoidObstacles->getValue())
  {
    if (fabs(targetAngleRads) < Math::degToRad(15))
    {
      // Try to keep a 'lane' free in front of the bot
      Vector2d left(ballPos + Vector2d(-laneWidth->getValue(), 0));
      Vector2d right(ballPos + Vector2d(laneWidth->getValue(), 0));

      Draw::line(Frame::Agent, Vector2d::Zero(), left, Colour::bgr::blue, 1, 0.4);
      Draw::line(Frame::Agent, Vector2d::Zero(), right, Colour::bgr::blue, 1, 0.4);

      auto leftDist = agentFrame->getOcclusionDistance(Math::angleToPoint(left));
      auto rightDist = agentFrame->getOcclusionDistance(Math::angleToPoint(right));

      if (!std::isnan(leftDist))
      {
        ySpeed += Math::clamp(1 - (leftDist/ballDist), 0.0, 1.0) * avoidSpeed->getValue();
        Draw::circleAtAngle(Frame::Agent, Math::angleToPoint(left), leftDist, 0.3, Colour::bgr::black, 1, 0.4);
      }
      if (!std::isnan(rightDist))
      {
        ySpeed -= Math::clamp(1 - (rightDist/ballDist), 0.0, 1.0) * avoidSpeed->getValue();
        Draw::circleAtAngle(Frame::Agent, Math::angleToPoint(right), rightDist, 0.3, Colour::bgr::black, 1, 0.4);
      }

      cout << "leftDist=" << leftDist << "\trightDist=" << rightDist << "\tballDist=" << ballDist << "\tySpeed=" << ySpeed << endl;
//      cout << "leftDist=" << leftDist << "\tballDist=" << ballDist << "\tySpeed=" << ySpeed << endl;
    }
  }

  d_walkModule->setMoveDir(xSpeed, ySpeed);
  d_walkModule->setTurnAngle(turnSpeed);

  writer.String("moveDir").StartArray().Double(xSpeed).Double(ySpeed).EndArray(2);
  writer.String("turn").Double(turnSpeed);

  return {};
}
