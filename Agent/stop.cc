#include "agent.hh"

#include "../DataStreamer/datastreamer.hh"
#include "../MotionLoop/motionloop.hh"
#include "../Voice/voice.hh"

using namespace bold;
using namespace std;

void Agent::requestStop()
{
  if (!isRunning())
    throw runtime_error("Not running");

  if (d_isShutdownRequested)
  {
    log::warning("Agent::requestStop") << "Forcing exit";
    exit(EXIT_FAILURE);
  }

  log::info("Agent::requestStop");

  // The option tree picks up this flag and performs a shutdown sequence
  d_isShutdownRequested = true;

  d_voice->say("Shutting down");
}


void Agent::onStopped()
{
  d_streamer->stop();

  if (d_motionLoop)
    d_motionLoop->stop();

  if (d_voice)
    d_voice->stop();

  log::info("Agent::stop") << "Stopped";
}
