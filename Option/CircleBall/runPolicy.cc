#include "circleball.ih"

OptionList CircleBall::runPolicy()
{
  double panAngle = MotionStatus::m_CurrentJoints.GetAngle(JointData::ID_HEAD_PAN);
  double panAngleRange = Head::GetInstance()->GetLeftLimitAngle();
  double panRatio = panAngle / panAngleRange;
  
  double x = 1;
  double y = panRatio < 0 ? 20 : -20;
  double a = panRatio < 0 ? -15 : 15;

  robotis::Walking::GetInstance()->m_Joint.SetEnableBodyWithoutHead(true, true);
  d_ambulator->setMoveDir(Eigen::Vector2d(x, y));
  d_ambulator->setTurnAngle(a);
  d_ambulator->step();

  return OptionList();
}
