#include "agent.ih"

void Agent::approachBall()
{
  auto ballObs = getBallObservation();

  if (ballObs == d_observations.end())
  {
    d_state = S_LOOK_FOR_BALL;
    return;
  }

  double pan = MotionStatus::m_CurrentJoints.GetAngle(JointData::ID_HEAD_PAN);
  double pan_range = Head::GetInstance()->GetLeftLimitAngle();
  double pan_percent = pan / pan_range;
  
  double tilt = MotionStatus::m_CurrentJoints.GetAngle(JointData::ID_HEAD_TILT);
  double tilt_min = Head::GetInstance()->GetBottomLimitAngle();
  double tilt_range = Head::GetInstance()->GetTopLimitAngle() - tilt_min;
  double tilt_percent = (tilt - tilt_min) / tilt_range;

  float maxKickAngle = 5.0;

  // Looking all the way down and straight ahead
  if (tilt <= tilt_min + MX28::RATIO_VALUE2ANGLE && fabs(pan) <= maxKickAngle)
  {
    d_state = S_LOOK_FOR_GOAL;
    return;
  }

  // Go for it
  //if (
  
  float followMaxFBStep = 30.0;
  float followMinFBStep = 5.0;
  float followMaxRLTurn = 35.0;
  float fitFBStep = 3.0;
  float fitMaxRLTurn = 35.0;
  float unitFBStep = 0.3;
  float unitRLTurn = 1.0;
  

}
