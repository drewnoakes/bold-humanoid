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
  double breakDist = 1.0;
  double alpha = dist/breakDist;
  
  Vector2d minMove(3.0, 0);
  Vector2d maxMove(10.0, 0);
  Vector2d move = minMove + alpha * (maxMove - minMove);
  
  d_ambulator->setMoveDir(move);
  
  double turnAngle = -atan2(ballPos->x(), ballPos->y()) * 35.0;
  double turnGain = 0.5;

  d_ambulator->setTurnAngle(turnGain * turnAngle);
  d_ambulator->step();


  return OptionList();
}
