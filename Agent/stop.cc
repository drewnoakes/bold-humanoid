#include "agent.ih"

void Agent::requestStop()
{
  if (!d_isRunning)
    throw new runtime_error("Not running");

  if (d_isStopRequested)
  {
    log::warning("Agent::requestStop") << "Forcing exit";
    exit(1);
  }

  log::info("Agent::requestStop");

  d_isStopRequested = true;
}

void Agent::stop()
{
  log::info("Agent::stop");
  d_isRunning = false;
  d_streamer->stop();

  if (d_motionLoop)
    d_motionLoop->stop();

  if (d_voice)
    d_voice->stop();
}
