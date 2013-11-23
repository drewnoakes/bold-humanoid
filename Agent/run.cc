#include "agent.ih"

void Agent::run()
{
  cout << "[Agent::run] Start" << endl;

  if (d_isRunning)
    throw new std::runtime_error("Already running");

  d_isRunning = true;

  std::stringstream announcement;
  announcement << "Player " << d_uniformNumber << " on team " << d_teamNumber;
  d_voice->say(announcement.str());

  if (d_haveBody)
  {
    d_cm730->torqueEnable(true);

    d_motionLoop->start();

    cout << "[Agent::run] Waiting for HardwareState" << endl;
    while (!AgentState::get<HardwareState>())
    {
      // Wait until the motion loop has read a hardware value
      usleep(8000);
    }
  }

  cout << ccolor::info << "[Agent::run] Starting think loop" << ccolor::reset << endl;

  while (d_isRunning)
  {
    think();
  }

  cout << ccolor::info << "[Agent::run] Stopped" << ccolor::reset << endl;
}
