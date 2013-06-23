#include "approachball.ih"

std::vector<std::shared_ptr<Option>> ApproachBall::runPolicy()
{
  auto ballPos = AgentState::get<AgentFrameState>()->getBallObservation();

  if (!ballPos)
  {
    cerr << "[ApproachBall::runPolicy] No ball observation!" << endl;
    return std::vector<std::shared_ptr<Option>>();
  }

  double dist = ballPos->head<2>().norm();

  double alpha = Math::clamp(dist/d_breakDist, 0.0, 1.0);

  Vector2d move = Math::lerp(alpha, Vector2d(d_minForwardSpeed, 0), Vector2d(d_maxForwardSpeed, 0));

  d_ambulator->setMoveDir(move);

  // NOTE atan has flipped x/y on purpose
  double turnAngle = -atan2(ballPos->x(), ballPos->y()) * d_turnScale;

  d_ambulator->setTurnAngle(turnAngle);

  return std::vector<std::shared_ptr<Option>>();
}
