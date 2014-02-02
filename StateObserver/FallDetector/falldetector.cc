#include "falldetector.hh"

#include "../../CM730Snapshot/cm730snapshot.hh"
#include "../../Voice/voice.hh"

#include <iostream>

using namespace bold;
using namespace std;

FallDetector::FallDetector(shared_ptr<Voice> voice)
: TypedStateObserver<HardwareState>("Fall detector", ThreadId::MotionLoop),
  d_voice(voice),
  d_windowSize(30),
  d_fbAvgValue(d_windowSize),
  d_forwardLimitValue(634),
  d_backwardLimitValue(444),
  d_fallenState(FallState::STANDUP)
{}

void FallDetector::observeTyped(std::shared_ptr<HardwareState const> const& hardwareState, SequentialTimer& timer)
{
  // Track the smoothed forward/backward acceleration to test for a consistent
  // indication that we have fallen.

  int avg = d_fbAvgValue.next(hardwareState->getCM730State().accRaw.y());

  // TODO this might not detect if we fall perfectly to the left/right

  if (d_fbAvgValue.count() == d_windowSize)
  {
    bool standingBefore = d_fallenState == FallState::STANDUP;

    // Update our estimate of whether we've fallen or not
    d_fallenState
      = avg > d_forwardLimitValue ? FallState::FORWARD
      : avg < d_backwardLimitValue ? FallState::BACKWARD
      : FallState::STANDUP;

    if (standingBefore && d_fallenState != FallState::STANDUP && d_voice->queueLength() == 0)
      d_voice->sayOneOf({"Ouch!", "Dammit", "Ooopsy", "Bah"});
  }
}
