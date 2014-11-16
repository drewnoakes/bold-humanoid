#include "agent.hh"

#include "../ImageLabeller/imagelabeller.hh"
#include "../MotionLoop/motionloop.hh"
#include "../util/log.hh"
#include "../Voice/voice.hh"

using namespace std;
using namespace bold;

void Agent::run()
{
  if (d_isRunning)
    throw runtime_error("Already running");

  d_isRunning = true;

  if (d_voice)
  {
    ostringstream announcement;
    announcement <<
      "Player " << (int)d_uniformNumber <<
      " on team " << (int)d_teamNumber <<
      ", " <<
      (d_teamColour == TeamColour::Cyan ? "cyan" : "magenta");
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
