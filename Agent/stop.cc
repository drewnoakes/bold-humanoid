#include "agent.ih"

void Agent::requestStop()
{
  if (!d_isRunning)
    throw new std::runtime_error("Not running");

  if (d_isStopRequested)
  {
    cerr << ccolor::warning << "[Agent::requestStop] Forcing exit" << ccolor::reset << endl;
    exit(1);
  }

  cout << "[Agent::requestStop]" << endl;

  d_isStopRequested = true;
}

void Agent::stop()
{
  cout << "[Agent::stop]" << endl;
  d_isRunning = false;
  d_motionLoop->stop();
}
