#include "agent.ih"

void Agent::run()
{
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

    log::info("Agent::run") << "Waiting for HardwareState before starting motion loop";
    while (!AgentState::get<HardwareState>())
    {
      // Wait until the motion loop has read a hardware value
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
