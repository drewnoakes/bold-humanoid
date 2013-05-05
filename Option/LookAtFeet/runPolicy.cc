#include "lookatfeet.ih"

OptionList LookAtFeet::runPolicy()
{
  Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);
  Head::GetInstance()->MoveToAngle(d_feetX, d_feetY);

  return OptionList();
}
