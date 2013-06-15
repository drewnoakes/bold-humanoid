#pragma once

#include "../option.hh"

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
      d_topAngle      = getParam("topAngle", -15.0);
      d_bottomAngle   = getParam("bottomAngle", 15.0);
      d_sideAngle     = getParam("sideAngle", 70.0);
      d_durationHoriz = getParam("durationHoriz", 3.0);
      d_durationVert  = getParam("durationVert", 0.4);
    }

    virtual std::vector<std::shared_ptr<Option>> runPolicy() override;

  private:
    std::shared_ptr<HeadModule> d_headModule;
    double d_topAngle;
    double d_bottomAngle;
    double d_sideAngle;
    double d_durationHoriz;
    double d_durationVert;
  };
}
