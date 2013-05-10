#pragma once

#include "../option.hh"
#include "../minIni/minIni.h"

namespace bold
{
  class HeadModule;

  class LookAtFeet : public Option
  {
  public:
    LookAtFeet(std::string const& id, minIni const& ini, std::shared_ptr<HeadModule> headModule)
    : Option(id),
      d_headModule(headModule)
    {
      d_feetX = ini.getd("LookAtFeet", "FeetX", 0);
      d_feetY = ini.getd("LookAtFeet", "FeetY", -67.5);
    }

    OptionList runPolicy() override;

  private:
    std::shared_ptr<HeadModule> d_headModule;
    double d_feetX;
    double d_feetY;
  };
}
