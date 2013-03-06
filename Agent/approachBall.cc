#include "agent.ih"

void Agent::approachBall()
{
  auto& vision = VisualCortex::getInstance();

  if (!vision.isBallVisible())
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
  if (tilt <= tilt_min + 8 && fabs(pan) <= maxKickAngle)
  {
    d_state = S_LOOK_FOR_GOAL;
    return;
  }

  // Default: full power scaled by tilt
  Vector2d moveDir(30.0, 0);
  moveDir = Vector2d(5.0, 0).cwiseMax(tilt_percent * moveDir);

  // Ball underneath center: small steps
  if (vision.ballObservation().pos.y() > d_camera->getPixelFormat().height / 2 + 10)
  {
    cout << "Fine tune walk!" << endl;
    moveDir.x() = 3.0;
  }

  double turnAngle = pan_percent * 35.0;

  d_ambulator.setMoveDir(moveDir);
  d_ambulator.setTurnAngle(turnAngle);
}
