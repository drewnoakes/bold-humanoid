#pragma once

#include "../option.hh"

#include "../../Config/config.hh"

namespace bold
{
  class HeadModule;

  class LookAround : public Option
  {
  public:
    LookAround(std::string const& id, std::shared_ptr<HeadModule> headModule, double sideAngle, std::function<double()> speedCallback = nullptr)
    : Option(id, "LookAround"),
      d_speedCallback(speedCallback),
      d_isResetNeeded(true),
      d_headModule(headModule),
      d_lastTimeSeconds(0)
    {
      d_topAngle      = Config::getSetting<double>("options.look-around.top-angle");
      d_bottomAngle   = Config::getSetting<double>("options.look-around.bottom-angle");
      d_sideAngle     = sideAngle; //Config::getSetting<double>("options.look-around.side-angle");
      d_durationHoriz = Config::getSetting<double>("options.look-around.horiz-duration");
      d_durationVert  = Config::getSetting<double>("options.look-around.vert-duration");
    }

    virtual std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

  private:
    std::function<double()> d_speedCallback;
    std::shared_ptr<HeadModule> d_headModule;

    /// The head's upwards tilt angle
    Setting<double>* d_topAngle;
    /// The head's downwards tilt angle
    Setting<double>* d_bottomAngle;
    /// The head's maximum pan angle (negated for left side)
    double d_sideAngle;

    /// The time spent in each horizontal movement
    Setting<double>* d_durationHoriz;
    /// The time spent in each vertical movement
    Setting<double>* d_durationVert;

    bool d_isResetNeeded;
    /// The last time this runPolicy was called
    double d_lastTimeSeconds;
    /// The time at which this option was considered started
    double d_startTimeSeconds;
  };
}
