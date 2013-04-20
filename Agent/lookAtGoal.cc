#include "agent.ih"

void Agent::lookAtGoal()
{
  auto const& goals = AgentState::get<CameraFrameState>()->getGoalObservations();

  if (goals.size() < 2)
  {
    return;
  }

  auto middle = (goals[0] + goals[1]) / 2;

  cout << "[Agent::lookAtGoal] middle: " << middle.transpose() << endl;

  lookAt(middle);
}
