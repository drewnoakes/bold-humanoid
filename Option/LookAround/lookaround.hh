#pragma once

#include "../option.hh"

namespace bold
{
  class HeadModule;

  class LookAround : public Option
  {
  public:
    LookAround(std::string const& id, std::shared_ptr<HeadModule> headModule, double sideAngle)
    : Option(id),
      d_headModule(headModule),
      d_lastTimeSeconds(0)
    {
      d_topAngle      = getParam("topAngle",     30.0);
      d_bottomAngle   = getParam("bottomAngle", -25.0);
      d_sideAngle     = sideAngle; //getParam("sideAngle",   100.0);
      d_durationHoriz = getParam("durationHoriz", 2.3);
      d_durationVert  = getParam("durationVert",  0.2);
    }

    virtual std::vector<std::shared_ptr<Option>> runPolicy() override;

  private:
    std::shared_ptr<HeadModule> d_headModule;

    /// The head's upwards tilt angle
    double d_topAngle;
    /// The head's downwards tilt angle
    double d_bottomAngle;
    /// The head's maximum pan angle (negated for left side)
    double d_sideAngle;

    /// The time spent in each horizontal movement
    double d_durationHoriz;
    /// The time spent in each vertical movement
    double d_durationVert;

    /// The last time this runPolicy was called
    double d_lastTimeSeconds;
    /// The time at which this option was considered started
    double d_startTimeSeconds;
  };
}
