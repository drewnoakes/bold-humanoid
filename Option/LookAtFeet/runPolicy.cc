#include "lookatfeet.ih"

std::vector<std::shared_ptr<Option>> LookAtFeet::runPolicy()
{
  d_headModule->moveToDegs(d_feetX, d_feetY);

  return std::vector<std::shared_ptr<Option>>();
}
