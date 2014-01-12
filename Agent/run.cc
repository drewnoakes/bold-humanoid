#include "agent.ih"

void Agent::run()
{
  if (d_isRunning)
    throw new runtime_error("Already running");

  d_isRunning = true;

  if (d_voice)
  {
    stringstream announcement;
    announcement << "Player " << d_uniformNumber << " on team " << d_teamNumber;
    d_voice->say(announcement.str());
  }

  if (d_haveBody)
  {
    d_motionLoop->start();

    while (!AgentState::get<HardwareState>())
    {
      // Wait until the motion loop has read a hardware value
      log::info("Agent::run") << "Waiting for HardwareState before starting think loop";
      usleep(8000);
    }
  }

  log::info("Agent::run") << "Starting think loop";

  while (d_isRunning)
  {
    think();
  }

  log::info("Agent::run") << "Stopped";
}
