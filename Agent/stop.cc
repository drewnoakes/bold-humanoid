#include "agent.ih"

void Agent::requestStop()
{
  if (!d_isRunning)
    throw runtime_error("Not running");

  if (d_isStopRequested)
  {
    log::warning("Agent::requestStop") << "Forcing exit";
    exit(EXIT_FAILURE);
  }

  log::info("Agent::requestStop");

  // The option tree picks up this flag and performs a shutdown sequence
  d_isStopRequested = true;

  d_voice->say("Shutting down");
}

void Agent::stop()
{
  log::verbose("Agent::stop") << "Stopping";

  d_isRunning = false;
  d_streamer->stop();

  if (d_motionLoop)
    d_motionLoop->stop();

  if (d_voice)
    d_voice->stop();

  log::info("Agent::stop") << "Stopped";
}
