#include "agent.ih"

void Agent::standUpIfFallen()
{
  if (d_autoGetUpFromFallen && MotionStatus::FALLEN != FallState::STANDUP)
  {
    auto walk = Walking::GetInstance();
    auto action = robotis::Action::GetInstance();
    auto head = Head::GetInstance();

    walk->Stop();

    // Loop until walking has stopped
    // TODO this blocks the think cycle, including image processing and localisation updates
    while(walk->IsRunning() == 1)
      usleep(8000);

    action->m_Joint.SetEnableBody(true, true);

    if (MotionStatus::FALLEN == FallState::FORWARD)
      action->Start((int)ActionPage::ForwardGetUp);
    else if (MotionStatus::FALLEN == FallState::BACKWARD)
      action->Start((int)ActionPage::BackwardGetUp);

    // Loop until the get up script has stopped
    // TODO this blocks the think cycle, including image processing and localisation updates
    while (action->IsRunning() == 1)
      usleep(8000);

    head->m_Joint.SetEnableHeadOnly(true, true);
    walk->m_Joint.SetEnableBodyWithoutHead(true, true);
  }
}