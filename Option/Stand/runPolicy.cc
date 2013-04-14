#include "stand.ih"

OptionList Stand::runPolicy()
{
  Robot::Action::GetInstance()->m_Joint.SetEnableBodyWithoutHead(true, true);

  d_ambulator->setMoveDir(Eigen::Vector2d(0,0));
  d_ambulator->setTurnAngle(0);
  d_ambulator->step();

  return OptionList();
}
