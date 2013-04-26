#include "lookatfeet.ih"

OptionList LookAtFeet::runPolicy()
{
  Robot::Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);
  Robot::Head::GetInstance()->MoveByAngle(d_feetX, d_feetY);

  return OptionList();
}
