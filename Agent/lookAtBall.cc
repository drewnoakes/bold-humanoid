#include "agent.ih"

void Agent::lookAtBall()
{
  auto& wm = WorldModel::getInstance();

  if (!wm.isBallVisible)
  {
    cerr << "[Agent::lookAtBall] No ball seen" << endl;
    return;
  }

  lookAt(wm.ballObservation.pos);
}
