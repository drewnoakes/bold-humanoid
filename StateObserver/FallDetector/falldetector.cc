#include "falldetector.hh"

#include "../../CM730Snapshot/cm730snapshot.hh"
#include "../../Config/config.hh"
#include "../../Voice/voice.hh"

#include <iostream>

using namespace bold;
using namespace std;

FallDetector::FallDetector(shared_ptr<Voice> voice)
: TypedStateObserver<HardwareState>("Fall detector", ThreadId::MotionLoop),
  d_voice(voice),
  d_fbAvgValue(Config::getStaticValue<int>("fall-detector.window-size")),
  d_forwardLimitValue(Config::getSetting<int>("fall-detector.forward-limit-value")),
  d_backwardLimitValue(Config::getSetting<int>("fall-detector.backward-limit-value")),
  d_fallenState(FallState::STANDUP)
{}

void FallDetector::observeTyped(std::shared_ptr<HardwareState const> const& hardwareState, SequentialTimer& timer)
{
  // Track the smoothed forward/backward acceleration to test for a consistent
  // indication that we have fallen.

  int avg = d_fbAvgValue.next(hardwareState->getCM730State().accRaw.y());

  // TODO this might not detect if we fall perfectly to the left/right

  if (d_fbAvgValue.isMature())
  {
    bool standingBefore = d_fallenState == FallState::STANDUP;

    // Update our estimate of whether we've fallen or not
    d_fallenState
      = avg > d_forwardLimitValue->getValue() ? FallState::FORWARD
      : avg < d_backwardLimitValue->getValue() ? FallState::BACKWARD
      : FallState::STANDUP;

    if (standingBefore && d_fallenState != FallState::STANDUP && d_voice->queueLength() == 0)
      d_voice->sayOneOf({"Ouch!", "Dammit", "Ooopsy", "Bah"});
  }
}
