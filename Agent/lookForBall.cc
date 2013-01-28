#include "agent.ih"

#include <sys/time.h>

void Agent::lookForBall()
{
  Robot::Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);

  auto ballObs = getBallObservation();

  bool ballSeen = ballObs != d_observations.end();

  if (ballSeen)
    d_ballSeenCnt++;
  else if (d_ballSeenCnt > 0)
    d_ballSeenCnt--;

  // Havent seen the ball enough
  if (ballSeen)
  {
    // Look at ball
    lookAtBall();

    if (d_ballSeenCnt >= 15)
      d_state = S_APPROACH_BALL;
  }
  else
  {
    // Oscillate
    double maxAmpH = 70.0;//d_ini.getd("Head Pan/Tilt", "left_limit", 80.0);
    double maxAmpV = 15.0;//
    timeval tval;
    gettimeofday(&tval, 0);
    
    double t = tval.tv_sec + tval.tv_usec / 1e6;

    static auto w = d_camera.get(CV_CAP_PROP_FRAME_WIDTH);
    static auto h = d_camera.get(CV_CAP_PROP_FRAME_HEIGHT);

    double periodH = 3.0;
    double periodV = 1.4;

    float hAngle = sin(t / periodH * 2.0 * M_PI) * maxAmpH;
    float vAngle = (sin(t / periodV * 2.0 * M_PI) + 1.0) * maxAmpV / 2.0;

    Head::GetInstance()->MoveByAngle(hAngle, vAngle);
  }

}
