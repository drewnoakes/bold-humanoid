#include "falldetector.hh"

#include "../../CM730Snapshot/cm730snapshot.hh"

#include <iostream>

using namespace bold;
using namespace std;

FallDetector::FallDetector()
: TypedStateObserver<HardwareState>("Fall detector", ThreadId::MotionLoop),
  d_windowSize(30),
  d_fbAvgValue(d_windowSize),
  d_forwardLimitValue(390),
  d_backwardLimitValue(580),
  d_fallenState(FallState::STANDUP)
{}

void FallDetector::observeTyped(std::shared_ptr<HardwareState const> hardwareState, SequentialTimer& timer)
{
  // Track the smoothed forward/backward acceleration to test for a consistent
  // indication that we have fallen.

  int avg = d_fbAvgValue.next(hardwareState->getCM730State()->accRaw.y());

  // TODO this might not detect if we fall perfectly to the left/right

  if (d_fbAvgValue.count() == d_windowSize)
  {
    // Update our estimate of whether we've fallen or not
    d_fallenState
      = avg < d_forwardLimitValue ? FallState::FORWARD
      : avg > d_backwardLimitValue ? FallState::BACKWARD
      : FallState::STANDUP;
  }
}
