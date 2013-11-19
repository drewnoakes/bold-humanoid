#include "lookatfeet.ih"

std::vector<std::shared_ptr<Option>> LookAtFeet::runPolicy()
{
  d_headModule->moveToDegs(d_panDegs->getValue(), d_tiltDegs->getValue());

  return std::vector<std::shared_ptr<Option>>();
}
