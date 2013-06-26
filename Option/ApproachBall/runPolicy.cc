#include "approachball.ih"

std::vector<std::shared_ptr<Option>> ApproachBall::runPolicy()
{
  assert(d_upperTurnLimitRads > d_lowerTurnLimitRads);
  assert(d_brakeDistance != 0);

  auto ballPos = AgentState::get<AgentFrameState>()->getBallObservation();

  if (!ballPos)
  {
    cerr << "[ApproachBall::runPolicy] No ball observation in AgentFrame yet ApproachBall was run" << endl;
    return std::vector<std::shared_ptr<Option>>();
  }

  double dist = ballPos->head<2>().norm();

  double speedDueToDistance = Math::clamp(dist/d_brakeDistance, 0.0, 1.0);

  // NOTE atan has flipped x/y on purpose
  double ballAngleRads = -atan2(ballPos->x(), ballPos->y());

  double speedScaleDueToAngle;

//   if (ballAngleRads > d_upperTurnLimitRads)
//   {
    // We have a long way to turn, so don't walk forwards
//     speedScaleDueToAngle = 0.0;
//   }
//   else if (ballAngleRads < d_lowerTurnLimitRads)
//   {
    // We are roughly facing the ball, so just walk forwards with full speed
//     speedScaleDueToAngle = 1.0;
//   }
//   else
  {
    speedScaleDueToAngle = Math::lerp(ballAngleRads, d_lowerTurnLimitRads, d_upperTurnLimitRads, 1.0, 0.0);
//     speedScaleDueToAngle = Math::clamp(fabs((ballAngleRads - d_lowerTurnLimitRads) /
//                                             (d_upperTurnLimitRads - d_lowerTurnLimitRads)), 0.0, 1.0);
  }

  cout << "speedScaleDueToAngle=" << speedScaleDueToAngle << " ballAngleRads=" << ballAngleRads << endl;

  Vector2d moveDir = Math::lerp(speedDueToDistance * speedScaleDueToAngle,
                                Vector2d(d_minForwardSpeed, 0),
                                Vector2d(d_maxForwardSpeed, 0));

  d_ambulator->setMoveDir(moveDir);
  d_ambulator->setTurnAngle(ballAngleRads * d_turnScale); // unspecified units

  return std::vector<std::shared_ptr<Option>>();
}
