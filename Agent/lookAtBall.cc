#include "agent.ih"

void Agent::lookAtBall()
{
  auto& vision = VisualCortex::getInstance();

  if (!vision.isBallVisible())
  {
    cerr << "[Agent::lookAtBall] No ball seen" << endl;
    return;
  }

  lookAt(vision.ballObservation().pos);
}
