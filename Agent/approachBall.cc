#include "agent.ih"

void Agent::approachBall()
{
  auto ballObs = getBallObservation();

  if (ballObs == d_observations.end())
  {
    d_state = S_LOOK_FOR_BALL;
    return;
  }

  lookAtBall();

  double pan = MotionStatus::m_CurrentJoints.GetAngle(JointData::ID_HEAD_PAN);
  double pan_range = Head::GetInstance()->GetLeftLimitAngle();
  double pan_percent = pan / pan_range;
  
  double tilt = MotionStatus::m_CurrentJoints.GetAngle(JointData::ID_HEAD_TILT);
  double tilt_min = Head::GetInstance()->GetBottomLimitAngle();
  double tilt_range = Head::GetInstance()->GetTopLimitAngle() - tilt_min;
  double tilt_percent = (tilt - tilt_min) / tilt_range;

  float maxKickAngle = 30.0;

  // Looking all the way down and straight ahead
  if (tilt <= tilt_min + MX28::RATIO_VALUE2ANGLE && fabs(pan) <= maxKickAngle)
  {
    d_state = S_LOOK_FOR_GOAL;
    return;
  }

  // Default: full power scaled by tilt
  Vector2d moveDir(30.0, 0);
  moveDir *= tilt_percent;

  // Ball underneath center: small steps
  if (ballObs->pos.y() < d_camera.get(CV_CAP_PROP_FRAME_HEIGHT) / 2 - 5)
    moveDir.x() = 3.0;

  double turnAngle = pan_percent * 35.0;

  d_ambulator.setMoveDir(moveDir);
  d_ambulator.setTurnAngle(turnAngle);
}
