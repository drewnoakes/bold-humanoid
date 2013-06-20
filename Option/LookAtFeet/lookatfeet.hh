#pragma once

#include "../option.hh"

namespace bold
{
  class HeadModule;

  class LookAtFeet : public Option
  {
  public:
    LookAtFeet(std::string const& id, std::shared_ptr<HeadModule> headModule)
    : Option(id),
      d_headModule(headModule)
    {
      d_feetX = getParam("feetX", 0);
      d_feetY = getParam("feetY", -67.5);
    }

    std::vector<std::shared_ptr<Option>> runPolicy() override;

  private:
    std::shared_ptr<HeadModule> d_headModule;
    double d_feetX;
    double d_feetY;
  };
}
