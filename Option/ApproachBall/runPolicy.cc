#include "approachball.ih"

vector<shared_ptr<Option>> ApproachBall::runPolicy(Writer<StringBuffer>& writer)
{
  assert(d_upperTurnLimitDegs->getValue() > d_lowerTurnLimitDegs->getValue());
  assert(d_brakeDistance->getValue() != 0);

  auto ballPos = State::get<AgentFrameState>()->getBallObservation();

  if (!ballPos)
  {
    writer.String("ball").Null();
//     log::warning("ApproachBall::runPolicy") << "No ball observation in AgentFrame yet ApproachBall was run";
    return vector<shared_ptr<Option>>();
  }

  writer.String("ballPos").StartArray().Double(ballPos->x()).Double(ballPos->y()).EndArray(2);

  double dist = ballPos->head<2>().norm();

  writer.String("ballDist").Double(dist);

  double speedDueToDistance = Math::clamp(dist/d_brakeDistance->getValue(), 0.0, 1.0);

  writer.String("distSpeed").Double(speedDueToDistance);

  // NOTE atan2 has flipped x/y on purpose
  double ballAngleRads = -atan2(ballPos->x(), ballPos->y());

  double speedScaleDueToAngle = Math::lerp(fabs(ballAngleRads), Math::degToRad(d_lowerTurnLimitDegs->getValue()), Math::degToRad(d_upperTurnLimitDegs->getValue()), 1.0, 0.0);

  writer.String("angleSpeed").Double(speedScaleDueToAngle);

  Vector2d moveDir = Math::lerp(speedDueToDistance * speedScaleDueToAngle,
                                Vector2d(d_minForwardSpeed->getValue(), 0),
                                Vector2d(d_maxForwardSpeed->getValue(), 0));

  // unspecified units
  double turnSpeed = ballAngleRads * d_turnScale->getValue();

  d_ambulator->setMoveDir(moveDir);
  d_ambulator->setTurnAngle(turnSpeed);

  writer.String("moveDir").StartArray().Double(moveDir.x()).Double(moveDir.y()).EndArray(2);

  writer.String("turn").Double(turnSpeed);

  return vector<shared_ptr<Option>>();
}
