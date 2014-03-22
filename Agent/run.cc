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

  if (!d_motionLoop->start())
  {
    log::error("Agent::run") << "Unable to start motion loop";
    throw runtime_error("Unable to start motion loop");
  }

  //
  // Become the think loop...
  //

  // Wait until the motion loop has read a hardware value
  log::info("Agent::run") << "Waiting for sufficient Motion Loop cycles";
  while (d_motionLoop->getCycleNumber() < 5)
    usleep(8000);

  log::info("Agent::run") << "Starting think loop";

  while (d_isRunning)
  {
    think();
  }

  log::info("Agent::run") << "Stopped";
}
