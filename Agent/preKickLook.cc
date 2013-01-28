#include "agent.ih"

void Agent::preKickLook()
{
  static double pklStartTime;

  stand();

  double tilt_min = Head::GetInstance()->GetBottomLimitAngle();

  Robot::Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);

  if (d_state == S_START_PREKICK_LOOK)
  {
    timeval now;
    gettimeofday(&now, 0);
    pklStartTime = now.tv_sec + now.tv_usec / 1e6;

    Head::GetInstance()->MoveByAngle(0, tilt_min);

    d_state = S_PREKICK_LOOK;
  }
  else
  {
    timeval now;
    gettimeofday(&now, 0);
    double t = now.tv_sec + now.tv_usec / 1e6;
    double dt = t - pklStartTime;

    if (dt >= 0.5)
    {
      auto ballObs = getBallObservation();
      if (ballObs == d_observations.end())
      {
	d_state = S_LOOK_FOR_BALL;
	return;
      }

      Robot::Action::GetInstance()->m_Joint.SetEnableBodyWithoutHead(true, true);

      static auto w = d_camera.get(CV_CAP_PROP_FRAME_WIDTH);

      cout << "KICKING!!! " << ballObs->pos.transpose() << " " << (w/2) << endl;

      if (ballObs->pos.x() < (w / 2))
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

      d_ballSeenCnt = d_ballSeenCnt = 0;
      d_state = S_LOOK_FOR_BALL;
    }
    
  }

}
