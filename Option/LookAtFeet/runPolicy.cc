#include "lookatfeet.ih"

OptionList LookAtFeet::runPolicy()
{
//   Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);
  d_headModule->moveToAngle(d_feetX, d_feetY);

  return OptionList();
}
