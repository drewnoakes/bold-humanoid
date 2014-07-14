#pragma once

#include "../option.hh"

#include "../../Config/config.hh"

namespace bold
{
  class HeadModule;

  class LookAround : public Option
  {
  public:
    static std::function<double(uint)> speedIfBallVisible(double scaleWhenVisible, double scaleWhenNotVisible = 1.0, double loopExp = 0.5);

    static double speedForLoop(uint loopCount, double loopExp = 0.0);

    LookAround(std::string const& id, std::shared_ptr<HeadModule> headModule, double sideAngle, std::function<double(uint)> speedCallback = nullptr);

    virtual std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    virtual void reset();

  private:
    std::function<double(uint)> d_speedCallback;
    std::shared_ptr<HeadModule> d_headModule;

    /// The head's maximum pan angle (negated for left side)
    double d_sideAngle;
    /// The head's upwards tilt angle
    Setting<double>* d_topAngle;
    /// The head's downwards tilt angle
    Setting<double>* d_bottomAngle;

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
    /// The number of full cycles the head has completed since the option was last reset. Zero during the first loop.
    uint d_loopCount;
  };
}
