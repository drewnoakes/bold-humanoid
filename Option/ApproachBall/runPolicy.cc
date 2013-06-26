#include "approachball.ih"

std::vector<std::shared_ptr<Option>> ApproachBall::runPolicy()
{
  auto ballPos = AgentState::get<AgentFrameState>()->getBallObservation();

  if (!ballPos)
  {
    cerr << "[ApproachBall::runPolicy] No ball observation in AgentFrame yet ApproachBall was run" << endl;
    return std::vector<std::shared_ptr<Option>>();
  }

  double dist = ballPos->head<2>().norm();

  double speedDueToDistance = Math::clamp(dist/d_breakDist, 0.0, 1.0);

  // NOTE atan has flipped x/y on purpose
  double ballAngleRads = -atan2(ballPos->x(), ballPos->y());

  double speedScaleDueToAngle = Math::clamp(fabs((ballAngleRads - d_lowerTurnLimitRads) /
                                                 (d_upperTurnLimitRads - d_lowerTurnLimitRads)), 0.0, 1.0);

  Vector2d moveDir = Math::lerp(speedDueToDistance * speedScaleDueToAngle,
                                Vector2d(d_minForwardSpeed, 0),
                                Vector2d(d_maxForwardSpeed, 0));

  d_ambulator->setMoveDir(moveDir);
  d_ambulator->setTurnAngle(ballAngleRads * d_turnScale); // unspecified units

  return std::vector<std::shared_ptr<Option>>();
}
