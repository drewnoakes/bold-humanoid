#include "approachball.ih"

vector<shared_ptr<Option>> ApproachBall::runPolicy(Writer<StringBuffer>& writer)
{
  ASSERT(d_upperTurnLimitDegs->getValue() > d_lowerTurnLimitDegs->getValue());
  ASSERT(d_brakeDistance->getValue() != 0);

  auto ballPos = State::get<AgentFrameState>()->getBallObservation();

  if (!ballPos)
  {
    writer.String("ball").Null();
//     log::warning("ApproachBall::runPolicy") << "No ball observation in AgentFrame yet ApproachBall was run";
    return {};
  }

  writer.String("ballPos").StartArray().Double(ballPos->x()).Double(ballPos->y()).EndArray(2);

  double dist = ballPos->head<2>().norm();

  d_behaviourControl->setPlayerActivity(
    dist < 0.5
      ? PlayerActivity::AttackingGoal
      : PlayerActivity::ApproachingBall);

  if (d_useCustomStopDistance)
  {
    dist -= d_stopDistance;
  }
  else
  {
    // subtract the stopping distance here so that the bot doesn't stop too suddenly!!!
    static auto stoppingDistance = Config::getSetting<double>("options.approach-ball.stop-distance");
    dist -= stoppingDistance->getValue();
  }

  if (dist < 0)
    dist = 0;

  writer.String("ballDist").Double(dist);

  double speedDueToDistance = Math::clamp(dist/d_brakeDistance->getValue(), 0.0, 1.0);

  writer.String("distSpeed").Double(speedDueToDistance);

  // NOTE atan2 has flipped x/y on purpose
  double ballAngleRads = -atan2(ballPos->x() - 0.04, ballPos->y());

  double speedScaleDueToAngle = Math::lerp(fabs(ballAngleRads),
                                           Math::degToRad(d_lowerTurnLimitDegs->getValue()),
                                           Math::degToRad(d_upperTurnLimitDegs->getValue()),
                                           1.0,
                                           0.0);

  writer.String("angleSpeed").Double(speedScaleDueToAngle);

  double xSpeed = Math::lerp(speedDueToDistance * speedScaleDueToAngle,
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

void ApproachBall::reset()
{
//   d_walkModule->reset();
}
