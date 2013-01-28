#include "agent.ih"

void Agent::lookAtGoal()
{
  if (d_goalObservations.size() < 2)
  {
    return;
  }


  auto middle = (d_goalObservations[0].pos + d_goalObservations[1].pos) / 2;

  cout << "middle: " << middle.transpose() << endl;

  lookAt(middle);
}
