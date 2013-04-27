#include "stand.ih"

OptionList Stand::runPolicy()
{
  Action::GetInstance()->m_Joint.SetEnableBodyWithoutHead(true, true);

  d_ambulator->setMoveDir(Eigen::Vector2d(0,0));
  d_ambulator->setTurnAngle(0);

  return OptionList();
}
