#include "lookatfeet.ih"

OptionList LookAtFeet::runPolicy()
{
  robotis::Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);
  robotis::Head::GetInstance()->MoveByAngle(d_feetX, d_feetY);

  return OptionList();
}
