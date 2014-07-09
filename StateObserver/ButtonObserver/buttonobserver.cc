#include "buttonobserver.hh"

#include "../../Agent/agent.hh"

using namespace bold;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ButtonTracker::observe(bool isPressed)
{
  if (isPressed == d_lastPressedState)
    return;

  if (isPressed)
  {
    // New press
    d_downAt = Clock::getTimestamp();
    d_isClaimed = false;
  }

  d_lastPressedState = isPressed;
}

bool ButtonTracker::isPressedForMillis(double millis)
{
  // Check if the button is down
  if (!d_lastPressedState)
    return false;

  // Check whether a client has already claimed this button press
  if (d_isClaimed)
    return false;

  // Check whether the button has been held for long enough
  if (Clock::getMillisSince(d_downAt) >= millis)
  {
    d_isClaimed = true;
    return true;
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ButtonObserver::ButtonObserver()
: TypedStateObserver<HardwareState>("Button observer", ThreadId::MotionLoop)
{}

std::shared_ptr<ButtonTracker> ButtonObserver::track(Button button)
{
  // TODO there is currently no way to stop tracking a button -- we don't need that yet though

  auto tracker = make_shared<ButtonTracker>(button);
  d_trackers.push_back(tracker);
  return tracker;
}

void ButtonObserver::observeTyped(shared_ptr<HardwareState const> const& hardwareState, SequentialTimer& timer)
{
  for (auto const& tracker : d_trackers)
  {
    tracker->observe(
      tracker->getButton() == Button::Left
        ? hardwareState->getCM730State().isModeButtonPressed
        : hardwareState->getCM730State().isStartButtonPressed);
  }
}
