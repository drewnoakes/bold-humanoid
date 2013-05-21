#include "agent.ih"

void Agent::stop()
{
  if (!d_isRunning)
    throw new std::runtime_error("Not running");

  cout << "[Agent::stop] Stopping..." << endl;

  d_isRunning = false;
  
  d_motionLoop->stop();
}
