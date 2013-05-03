#pragma once

#include "../option.hh"
#include <minIni.h>

namespace bold
{
  class LookAtFeet : public Option
  {
  public:
    LookAtFeet(std::string const& id, minIni const& ini)
    : Option(id)
    {
      d_feetX = ini.getd("LookAtFeet", "FeetX", 0);
      d_feetY = ini.getd("LookAtFeet", "FeetY", -67.5);
    }

    OptionList runPolicy() override;

  private:
    double d_feetX;
    double d_feetY;
  };
}
