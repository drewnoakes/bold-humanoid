#include "agent.ih"

void Agent::lookAtGoal()
{
  auto& vision = VisualCortex::getInstance();

  if (vision.goalObservations().size() < 2)
  {
    return;
  }

  auto middle = (vision.goalObservations()[0].pos + vision.goalObservations()[1].pos) / 2;

  cout << "[Agent::lookAtGoal] middle: " << middle.transpose() << endl;

  lookAt(middle);
}
