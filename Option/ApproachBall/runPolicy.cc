#include "approachball.ih"

OptionList ApproachBall::runPolicy()
{
  auto agentFrameState = AgentState::get<AgentFrameState>();

  auto ballPos = agentFrameState->getBallObservation();
  if (!ballPos)
  {
    cerr << "[ApproachBall::runPolicy] No ball observation! " << endl;
    return OptionList();
  }

  double dist = ballPos->head<2>().norm();
  double breakDist = 0.5;
  double alpha = dist/breakDist;
  if (alpha > 1)
    alpha = 1;
  
  Vector2d minMove(5.0, 0);
  Vector2d maxMove(30.0, 0);
  Vector2d move = minMove + alpha * (maxMove - minMove);
  
  d_ambulator->setMoveDir(move);
  
  double turnAngle = -atan2(ballPos->x(), ballPos->y()) * 35.0;
  double turnGain = 0.5;

  d_ambulator->setTurnAngle(turnGain * turnAngle);
  d_ambulator->step();


  return OptionList();
}
