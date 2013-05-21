#pragma once

#include "../option.hh"
#include "../minIni/minIni.h"

namespace bold
{
  class HeadModule;

  class LookAround : public Option
  {
  public:
    LookAround(std::string const& id, std::shared_ptr<HeadModule> headModule)
    : Option(id),
      d_headModule(headModule)
    {
      d_topAngle      = -15.0;//ini.getd("LookAround", "TopAngle", -15.0);
      d_bottomAngle   = 15.0;//ini.getd("LookAround", "BottomAngle", 15.0);
      d_sideAngle     = 7.0;//ini.getd("LookAround", "SideAngle", 70.0);
      d_durationHoriz = 3.0;//ini.getd("LookAround", "DurationHoriz", 3.0);
      d_durationVert  = 0.4;//ini.getd("LookAround", "DurationVert", 0.4);
    }

    virtual OptionList runPolicy() override;

  private:
    std::shared_ptr<HeadModule> d_headModule;
    double d_topAngle;
    double d_bottomAngle;
    double d_sideAngle;
    double d_durationHoriz;
    double d_durationVert;
  };
}
