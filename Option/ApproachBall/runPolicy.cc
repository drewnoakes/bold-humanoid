#include "approachball.ih"

OptionList ApproachBall::runPolicy()
{
  auto ballPos = AgentState::get<AgentFrameState>()->getBallObservation();

  if (!ballPos)
  {
    cerr << "[ApproachBall::runPolicy] No ball observation!" << endl;
    return OptionList();
  }

  double dist = ballPos->head<2>().norm();
  double breakDist = 0.5;

  double alpha = Math::clamp(dist/breakDist, 0.0, 1.0);

  Vector2d move = Math::lerp(alpha, Vector2d(5.0, 0), Vector2d(30.0, 0));
  d_ambulator->setMoveDir(move);

  double turnAngle = -atan2(ballPos->x(), ballPos->y()) * 35.0;
  double turnGain = 0.5;

  d_ambulator->setTurnAngle(turnGain * turnAngle);
  d_ambulator->step();

  return OptionList();
}
