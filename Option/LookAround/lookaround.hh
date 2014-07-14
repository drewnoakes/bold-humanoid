#pragma once

#include "../option.hh"
#include "../../Clock/clock.hh"

namespace bold
{
  class HeadModule;
  template<typename> class Setting;

  struct LookAroundStage
  {
    LookAroundStage(double durationSeconds, double tiltAngle, double panAngle)
    : durationSeconds(durationSeconds), tiltAngle(tiltAngle), panAngle(panAngle)
    {}

    double durationSeconds;
    double tiltAngle;
    double panAngle;
  };

  class LookAround : public Option
  {
  public:
    static std::function<double(uint)> speedIfBallVisible(double scaleWhenVisible, double scaleWhenNotVisible = 1.0, double loopExp = 0.5);

    static double speedForLoop(uint loopCount, double loopExp = 0.0);

    LookAround(std::string const& id, std::shared_ptr<HeadModule> headModule, double sideAngle, std::function<double(uint)> speedCallback = nullptr);

    virtual std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    virtual void reset();

    void setPhase(double phase);

    double getPhase() const { return d_phase; };

  private:
    std::vector<LookAroundStage> d_stages;
    std::function<double(uint)> d_speedCallback;
    std::shared_ptr<HeadModule> d_headModule;

    /// The amount the pan speed is to be increased per step after being lowered by the speed callback
    Setting<double>* d_speedStep;

    Clock::Timestamp d_lastTime;
    double d_lastSpeed;
    double d_phase;

    /// The number of full cycles the head has completed since the option was last reset. Zero during the first loop.
    uint d_loopCount;
  };
}
