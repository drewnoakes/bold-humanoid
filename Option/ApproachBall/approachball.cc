#include "approachball.hh"

#include "../../BehaviourControl/behaviourcontrol.hh"
#include "../../Drawing/drawing.hh"
#include "../../MotionModule/WalkModule/walkmodule.hh"
#include "../../State/state.hh"
#include "../../StateObject/AgentFrameState/agentframestate.hh"
#include "../../geometry/Polygon2.hh"

using namespace bold;
using namespace bold::Colour;
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

  Vector2d ballPos = agentFrame->getBallObservation()->head<2>();

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
  static auto occlusionBrakeDistance = Config::getSetting<double>("options.approach-ball.avoid-obstacles.brake-distance");
  static auto minForwardSpeedScale = Config::getSetting<double>("options.approach-ball.avoid-obstacles.min-fwd-scale");
  if (avoidObstacles->getValue())
  {
    // Determine the polygon of the direct lane to the ball
    Vector2d perp(Math::findPerpendicularVector(target).normalized() * (laneWidth->getValue()/2.0));

    Polygon2d::PointVector lanePoints;
    lanePoints.push_back(target + perp);
    lanePoints.push_back(perp);
    lanePoints.push_back(-perp);
    lanePoints.push_back(target - perp);

    Polygon2d lanePoly(lanePoints);

    Draw::polygon(Frame::Agent, lanePoints, bgr::blue, 0.1, bgr::blue, 0.5, 1.5);

    // Iterate through the occlusion rays
    double minDistInLane = numeric_limits<double>::max();
    for (auto const& ray : agentFrame->getOcclusionRays())
    {
      if (lanePoly.contains(ray.near()))
      {
        Draw::circle(Frame::Agent, ray.near(), 0.02, bgr::lightBlue, 1, 0.8);

        if ((ray.near() - ballPos).norm() > FieldMap::getBallDiameter() * 1.5)
          minDistInLane = std::min(minDistInLane, ray.near().norm());
      }
    }

    double brakeDist = occlusionBrakeDistance->getValue();
    if (minDistInLane < brakeDist)
      xSpeed *= min(brakeDist - minDistInLane, minForwardSpeedScale->getValue());

    if (fabs(targetAngleRads) < Math::degToRad(15))
    {
      // Try to keep a 'lane' free in front of the bot
      Vector2d left(ballPos + Vector2d(-laneWidth->getValue(), 0));
      Vector2d right(ballPos + Vector2d(laneWidth->getValue(), 0));

      Draw::line(Frame::Agent, Vector2d::Zero(), left, bgr::blue, 1, 0.4);
      Draw::line(Frame::Agent, Vector2d::Zero(), right, bgr::blue, 1, 0.4);

      auto leftDist = agentFrame->getOcclusionDistance(Math::angleToPoint(left));
      auto rightDist = agentFrame->getOcclusionDistance(Math::angleToPoint(right));

      if (!std::isnan(leftDist))
      {
        ySpeed += Math::clamp(1 - (leftDist/ballDist), 0.0, 1.0) * avoidSpeed->getValue();
        Draw::circleAtAngle(Frame::Agent, Math::angleToPoint(left), leftDist, 0.3, bgr::black, 1, 0.4);
      }
      if (!std::isnan(rightDist))
      {
        ySpeed -= Math::clamp(1 - (rightDist/ballDist), 0.0, 1.0) * avoidSpeed->getValue();
        Draw::circleAtAngle(Frame::Agent, Math::angleToPoint(right), rightDist, 0.3, bgr::black, 1, 0.4);
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
