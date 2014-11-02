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
  static auto turnScale          = Config::getSetting<double>("options.approach-ball.turn-speed-scale");
  static auto maxForwardSpeed    = Config::getSetting<double>("options.approach-ball.max-forward-speed");
  static auto minForwardSpeed    = Config::getSetting<double>("options.approach-ball.min-forward-speed");
  static auto brakeDistance      = Config::getSetting<double>("options.approach-ball.brake-distance");
  static auto lowerTurnLimitDegs = Config::getSetting<double>("options.approach-ball.lower-turn-limit-degs");
  static auto upperTurnLimitDegs = Config::getSetting<double>("options.approach-ball.upper-turn-limit-degs");

  ASSERT(upperTurnLimitDegs->getValue() > lowerTurnLimitDegs->getValue());
  ASSERT(brakeDistance->getValue() != 0);

  auto agentFrame = State::get<AgentFrameState>();

  if (!agentFrame->isBallVisible())
  {
    writer.String("ballPos");
    writer.Null();
    return {};
  }

  //
  // WALK TOWARDS BALL
  //

  Vector2d ballPos = agentFrame->getBallObservation()->head<2>();

  writer.String("ballPos");
  writer.StartArray();
  writer.Double(ballPos.x(), "%.3f");
  writer.Double(ballPos.y(), "%.3f");
  writer.EndArray(2);

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

  writer.String("ballDist");
  writer.Double(ballDist, "%.3f");

  writer.String("walkDist");
  writer.Double(walkDist, "%.3f");

  double speedScaleDueToDistance = Math::clamp(walkDist/brakeDistance->getValue(), 0.0, 1.0);

  writer.String("distSpeed");
  writer.Double(speedScaleDueToDistance, "%.3f");

  Vector2d target = ballPos + Vector2d(0.04, 0);
  double ballAngleRads = Math::angleToPoint(ballPos);
  double targetAngleRads = Math::angleToPoint(target);

  double speedScaleDueToAngle = Math::lerp(fabs(targetAngleRads),
                                           Math::degToRad(lowerTurnLimitDegs->getValue()),
                                           Math::degToRad(upperTurnLimitDegs->getValue()),
                                           1.0,
                                           0.0);

  writer.String("angleSpeed");
  writer.Double(speedScaleDueToAngle, "%.3f");

  double xSpeedScale = speedScaleDueToDistance * speedScaleDueToAngle;
  ASSERT(xSpeedScale >= 0.0);
  ASSERT(xSpeedScale <= 1.0);
  double xSpeed = Math::lerp(xSpeedScale,
                             minForwardSpeed->getValue(),
                             maxForwardSpeed->getValue());

  ASSERT(xSpeed >= minForwardSpeed->getValue());

  // unspecified units
  double turnSpeed = targetAngleRads * turnScale->getValue();

  double ySpeed = 0;

  //
  // AVOID OBSTACLES
  //

  static auto avoidObstacles         = Config::getSetting<bool>("options.approach-ball.avoid-obstacles.enabled");
  static auto laneWidth              = Config::getSetting<double>("options.approach-ball.avoid-obstacles.lane-width");
  static auto occlusionBrakeDistance = Config::getSetting<double>("options.approach-ball.avoid-obstacles.brake-distance");
  static auto minForwardSpeedScale   = Config::getSetting<double>("options.approach-ball.avoid-obstacles.min-fwd-scale");
  static auto avoidTurnSpeed         = Config::getSetting<double>("options.approach-ball.avoid-obstacles.turn-speed");
  static auto sideStepSpeed          = Config::getSetting<double>("options.approach-ball.avoid-obstacles.side-step-speed");
  static auto ignoreNearBallDistance = Config::getSetting<double>("options.approach-ball.avoid-obstacles.ignore-near-ball-dist");

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

    Draw::fillPolygon(Frame::Agent, lanePoly, bgr::blue, 0.1, bgr::blue, 0.5, 1.5);

    // Iterate through the occlusion rays
    double minDistInLane = numeric_limits<double>::max();
    double leftPenaltySum = 0, rightPenaltySum = 0;
    for (auto const& ray : agentFrame->getOcclusionRays())
    {
      // Only consider occlusion points within our lane
      if (!lanePoly.contains(ray.near()))
        continue;

      // Ignore occlusion points which are quite close to the ball
      if (agentFrame->isNearBall(ray.near(), ignoreNearBallDistance->getValue()))
        continue;

      // Draw for debugging
      Draw::circle(Frame::Agent, ray.near(), 0.02, bgr::lightBlue, 1, 0.8);

      // Remember the minimum distance to an obstacle in the lane
      minDistInLane = std::min(minDistInLane, ray.near().norm());

      // Turn penalty
      double penalty = Math::clamp((ballDist - ray.near().norm()) / ballDist, 0.0, 1.0);
      if (Math::angleToPoint(ray.near()) < ballAngleRads)
        leftPenaltySum += penalty;
      else
        rightPenaltySum += penalty;
    }

    // Check there's a difference between sides (also caters for zeroes which causes NaN)
    if (leftPenaltySum != rightPenaltySum)
    {
      // Turn away from the occluded side
      double turnRatio = leftPenaltySum < rightPenaltySum
        ? -1 + (leftPenaltySum / rightPenaltySum)
        :  1 - (rightPenaltySum / leftPenaltySum);
      turnSpeed += turnRatio * avoidTurnSpeed->getValue();
      ySpeed += turnRatio * sideStepSpeed->getValue();
    }

    // Slow down if there is an occlusion close in front of us
    double brakeDist = occlusionBrakeDistance->getValue();
    if (minDistInLane < brakeDist)
      xSpeed *= max(minDistInLane / brakeDist, minForwardSpeedScale->getValue());
  }

  ASSERT(xSpeed <= maxForwardSpeed->getValue());

  d_walkModule->setMoveDir(xSpeed, ySpeed);
  d_walkModule->setTurnAngle(turnSpeed);

  writer.String("moveDir");
  writer.StartArray();
  writer.Double(xSpeed, "%.3f");
  writer.Double(ySpeed, "%.3f");
  writer.EndArray(2);

  writer.String("turn");
  writer.Double(turnSpeed, "%.3f");

  return {};
}
