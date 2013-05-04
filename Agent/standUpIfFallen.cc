#include "agent.ih"

void Agent::standUpIfFallen()
{
  if (d_autoGetUpFromFallen && MotionStatus::FALLEN != STANDUP)
  {
    Walking::GetInstance()->Stop();
    while(Walking::GetInstance()->IsRunning() == 1) usleep(8000);

    robotis::Action::GetInstance()->m_Joint.SetEnableBody(true, true);

    if (MotionStatus::FALLEN == FORWARD)
      robotis::Action::GetInstance()->Start(10);   // FORWARD GETUP
    else if (MotionStatus::FALLEN == BACKWARD)
      robotis::Action::GetInstance()->Start(11);   // BACKWARD GETUP

    while (robotis::Action::GetInstance()->IsRunning() == 1) usleep(8000);

    Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);
    Walking::GetInstance()->m_Joint.SetEnableBodyWithoutHead(true, true);
  }
}