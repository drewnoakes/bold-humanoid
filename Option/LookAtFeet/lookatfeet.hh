#pragma once

#include "../option.hh"
#include "../minIni/minIni.h"

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
      d_feetX = 0;//ini.getd("LookAtFeet", "FeetX", 0);
      d_feetY = -67.5;//ini.getd("LookAtFeet", "FeetY", -67.5);
    }

    OptionList runPolicy() override;

  private:
    std::shared_ptr<HeadModule> d_headModule;
    double d_feetX;
    double d_feetY;
  };
}
