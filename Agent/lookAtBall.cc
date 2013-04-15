#include "agent.ih"

void Agent::lookAtBall()
{
  auto const& ballObs = AgentState::getInstance().get<CameraFrameState>()->getBallObservation();

  if (!ballObs.hasValue())
  {
    cerr << "[Agent::lookAtBall] No ball seen" << endl;
    return;
  }

  lookAt(*ballObs.value());
}
