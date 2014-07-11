#include "walkto.hh"

#include "../../Config/config.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

WalkTo::WalkTo(string const& id, shared_ptr<WalkModule> walkModule)
  : Option{id, "WalkTo"},
  d_targetPos{0.0, 0.0},
  d_targetAngle{0.0},
  d_turnDist{0.5},
  d_walkModule{walkModule}
{
  d_turnScale          = Config::getSetting<double>("options.walkto.turn-speed-scale");
  d_maxForwardSpeed    = Config::getSetting<double>("options.walkto.max-forward-speed");
  d_minForwardSpeed    = Config::getSetting<double>("options.walkto.min-forward-speed");
  d_maxSidewaysSpeed   = Config::getSetting<double>("options.walkto.max-sideways-speed");
  d_minSidewaysSpeed   = Config::getSetting<double>("options.walkto.min-sideways-speed");
  d_brakeDistance      = Config::getSetting<double>("options.walkto.brake-distance");
  d_lowerTurnLimitDegs = Config::getSetting<double>("options.walkto.lower-turn-limit-degs");
  d_upperTurnLimitDegs = Config::getSetting<double>("options.walkto.upper-turn-limit-degs");

}

Option::OptionVector WalkTo::runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer)
{
  ASSERT(d_upperTurnLimitDegs->getValue() > d_lowerTurnLimitDegs->getValue());
  ASSERT(d_brakeDistance->getValue() != 0);

  double walkDist = d_targetPos.norm();

  static auto stoppingDistance = Config::getSetting<double>("options.approach-ball.stop-distance");

  writer.String("walkDist").Double(walkDist);

  double speedScaleDueToDistance = Math::clamp(walkDist / d_brakeDistance->getValue(), 0.0, 1.0);

  writer.String("distSpeed").Double(speedScaleDueToDistance);

  double faceAngle = 0;
  if (walkDist < d_turnDist)
    faceAngle = d_targetAngle;
  else
  {
    faceAngle = Math::angleToPoint(d_targetPos);
    if (fabs(faceAngle) > M_PI / 2.0)
      faceAngle = Math::normaliseRads(faceAngle + M_PI);
  }

  double speedScaleDueToAngle = Math::lerp(fabs(faceAngle),
                                           Math::degToRad(d_lowerTurnLimitDegs->getValue()),
                                           Math::degToRad(d_upperTurnLimitDegs->getValue()),
                                           1.0,
                                           0.0);

  writer.String("angleSpeed").Double(speedScaleDueToAngle);

  double xSpeed = Math::lerp(speedScaleDueToDistance * speedScaleDueToAngle * d_targetPos.y(),
                             d_minForwardSpeed->getValue(),
                             d_maxForwardSpeed->getValue());
  
  double ySpeed = -Math::lerp(speedScaleDueToDistance * speedScaleDueToAngle * d_targetPos.x(),
                             d_minSidewaysSpeed->getValue(),
                             d_maxSidewaysSpeed->getValue());
  
  // unspecified units
  double turnSpeed = faceAngle * d_turnScale->getValue();

  d_walkModule->setMoveDir(xSpeed, ySpeed);
  d_walkModule->setTurnAngle(turnSpeed);

  writer.String("moveDir").StartArray().Double(xSpeed).Double(ySpeed).EndArray(2);

  writer.String("turn").Double(turnSpeed);

  return {};

}


