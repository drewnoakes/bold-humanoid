#include "lookatfeet.ih"

std::vector<std::shared_ptr<Option>> LookAtFeet::runPolicy()
{
//   HeadModule::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);
  d_headModule->moveToAngle(d_feetX, d_feetY);

  return std::vector<std::shared_ptr<Option>>();
}
