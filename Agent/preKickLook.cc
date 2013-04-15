#include "agent.ih"

void Agent::preKickLook()
{
  static double pklStartTime;

  //stand();

  double tilt_min = Head::GetInstance()->GetBottomLimitAngle();

  Robot::Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);

  if (d_state == State::S_START_PREKICK_LOOK)
  {
    timeval now;
    gettimeofday(&now, 0);
    pklStartTime = now.tv_sec + now.tv_usec / 1e6;

    Head::GetInstance()->MoveByAngle(0, tilt_min);

    d_state = State::S_PREKICK_LOOK;
  }
  else
  {
    timeval now;
    gettimeofday(&now, 0);
    double t = now.tv_sec + now.tv_usec / 1e6;
    double dt = t - pklStartTime;

    if (dt >= 0.5)
    {
      auto const& cameraFrame = AgentState::getInstance().get<CameraFrameState>();

      if (!cameraFrame->isBallVisible())
      {
        d_state = State::S_LOOK_FOR_BALL;
        return;
      }

      Robot::Action::GetInstance()->m_Joint.SetEnableBodyWithoutHead(true, true);

      static auto w = d_camera->getPixelFormat().width;

      auto const& ballObs = cameraFrame->getBallObservation().value();

      cout << "KICKING!!! " << ballObs->transpose() << " " << (w/2) << endl;

      if (ballObs->x() < (w / 2))
      {
        while(Robot::Action::GetInstance()->Start("rk") == false)
          usleep(8000);
        while(Robot::Action::GetInstance()->IsRunning())
          usleep(8*1000);
      }
      else
      {
        while(Robot::Action::GetInstance()->Start("lk") == false)
          usleep(8000);
        while(Robot::Action::GetInstance()->IsRunning())
          usleep(8*1000);
      }

      d_ballSeenCnt = d_goalSeenCnt = 0;
      d_state = State::S_LOOK_FOR_BALL;
    }
  }
}
