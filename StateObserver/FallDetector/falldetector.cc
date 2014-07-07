#include "falldetector.hh"

#include "../../Voice/voice.hh"
#include "../../JointId/jointid.hh"
#include "../../StateObject/HardwareState/hardwarestate.hh"
#include "../../Config/config.hh"

#include <iomanip>

using namespace bold;
using namespace std;

typedef unsigned char uchar;

FallDetector::FallDetector(std::shared_ptr<Voice> voice)
: d_voice(voice),
  d_fallenState(FallState::STANDUP),
  d_startTime(Clock::getTimestamp())
{}

void FallDetector::setFallState(FallState fallState)
{
  // TODO add some hysteresis here to avoid flickering between states (seen as multiple consecutive fall-data log entries)

  bool standingBefore = d_fallenState == FallState::STANDUP;

  d_fallenState = fallState;

  if (standingBefore && fallState != FallState::STANDUP)
  {
    auto const& hardwareState = State::get<HardwareState>();

    ASSERT(hardwareState);

    // Log a bunch of data when a fall is detected
    stringstream msg;
    msg << setprecision(3) << Clock::getSecondsSince(d_startTime) << ","
      << getFallStateName(fallState) << ","
      << hardwareState->getCM730State().voltage;

    for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      msg << "," << (int)hardwareState->getMX28State(jointId).presentTemp;

    msg << ",";
    logFallData(msg);

    log::info("fall-data") << msg.str();

    // Announce the fall
    if (d_voice->queueLength() == 0)
      d_voice->sayOneOf({"Ouch", "Dammit", "Ooopsy", "Bah", "Why me", "Not again", "That hurt"});
  }
}

string bold::getFallStateName(FallState fallState)
{
  switch (fallState)
  {
    case FallState::STANDUP:
      return "Standup";
    case FallState::BACKWARD:
      return "Backward";
    case FallState::FORWARD:
      return "Forward";
    case FallState::LEFT:
      return "Left";
    case FallState::RIGHT:
      return "Right";
    default:
      return "Unknown";
  }
}
