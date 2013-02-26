#include "agent.ih"

void Agent::lookAtGoal()
{
  auto& wm = WorldModel::getInstance();

  if (wm.goalObservations.size() < 2)
  {
    return;
  }

  auto middle = (wm.goalObservations[0].pos + wm.goalObservations[1].pos) / 2;

  cout << "[Agent::lookAtGoal] middle: " << middle.transpose() << endl;

  lookAt(middle);
}
