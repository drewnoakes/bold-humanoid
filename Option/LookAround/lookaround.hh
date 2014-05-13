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
      d_headModule(headModule),
      d_isResetNeeded(true),
      d_lastTimeSeconds(0),
      d_speed(1.0)
    {
      d_topAngle      = Config::getSetting<double>("options.look-around.top-angle");
      d_bottomAngle   = Config::getSetting<double>("options.look-around.bottom-angle");
      d_sideAngle     = sideAngle;
      d_durationHorizUpper = Config::getSetting<double>("options.look-around.horiz-duration-upper");
      d_durationHorizLower = Config::getSetting<double>("options.look-around.horiz-duration-lower");
      d_durationVert  = Config::getSetting<double>("options.look-around.vert-duration");
      d_speedStep     = Config::getSetting<double>("options.look-around.speed-step");
    }

    virtual std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    virtual void reset() override { d_isResetNeeded = true; }

  private:
    std::function<double()> d_speedCallback;
    std::shared_ptr<HeadModule> d_headModule;

    /// The head's upwards tilt angle
    Setting<double>* d_topAngle;
    /// The head's downwards tilt angle
    Setting<double>* d_bottomAngle;
    /// The head's maximum pan angle (negated for left side)
    double d_sideAngle;

    /// The time spent in the upper horizontal movement
    Setting<double>* d_durationHorizUpper;
    /// The time spent in the lower horizontal movement
    Setting<double>* d_durationHorizLower;
    /// The time spent in each vertical movement
    Setting<double>* d_durationVert;
    /// The amount the pan speed is to be increased per step after being lowered by the speed callback
    Setting<double>* d_speedStep;

    bool d_isResetNeeded;
    /// The last time this runPolicy was called
    double d_lastTimeSeconds;
    /// The time at which this option was considered started
    double d_startTimeSeconds;
    /// Scalar for the speed of movement, between 0 and 1.
    double d_speed;
  };
}
