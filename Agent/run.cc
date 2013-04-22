#include "agent.ih"

void Agent::run()
{
  cout << "[Agent::run] Start" << endl;

  if (d_isRunning)
    throw new std::runtime_error("Already running");

  d_isRunning = true;

  while (d_isRunning)
  {
    think();
  }

  // TODO if the agent is walking, allow it to come to a stable pose before terminating

  cout << "[Agent::run] Stopped" << endl;
}
