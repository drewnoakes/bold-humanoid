#include "agent.ih"

void Agent::standUpIfFallen()
{
  // TODO this should be part of the behaviour tree

  // TODO use OrientationState instead of d_fallDetector (which will be observing the hardware snapshots)

  if (!d_autoGetUpFromFallen || d_fallDetector->getFallenState() == FallState::STANDUP)
    return;

  // TODO arbitrate this differently
  d_walkModule->stop();

  // Loop until walking has stopped
  // TODO this blocks the think cycle, including image processing and localisation updates
  while (d_walkModule->isRunning())
    usleep(8000);

  if (d_fallDetector->getFallenState() == FallState::FORWARD)
    d_actionModule->start((int)ActionPage::ForwardGetUp);
  else if (d_fallDetector->getFallenState() == FallState::BACKWARD)
    d_actionModule->start((int)ActionPage::BackwardGetUp);

  // Loop until the get up script has stopped
  // TODO this blocks the think cycle, including image processing, localisation, data-streamer updates
  while (d_actionModule->isRunning())
    usleep(8000);
}
