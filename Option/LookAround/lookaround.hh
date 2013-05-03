#pragma once

#include "../option.hh"
#include <minIni.h>

namespace bold
{
  class LookAround : public Option
  {
  public:
    LookAround(std::string const& id, minIni const& ini)
    : Option(id)
    {
      d_topAngle      = ini.getd("LookAround", "TopAngle", -15.0);
      d_bottomAngle   = ini.getd("LookAround", "BottomAngle", 15.0);
      d_sideAngle     = ini.getd("LookAround", "SideAngle", 70.0);
      d_durationHoriz = ini.getd("LookAround", "DurationHoriz", 3.0);
      d_durationVert  = ini.getd("LookAround", "DurationVert", 0.4);
    }

    virtual OptionList runPolicy() override;

  private:
    double d_topAngle;
    double d_bottomAngle;
    double d_sideAngle;
    double d_durationHoriz;
    double d_durationVert;
  };
}
