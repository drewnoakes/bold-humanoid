#pragma once

#include "../../Config/config.hh"
#include "../../Math/math.hh"
#include "../option.hh"

namespace bold
{
  class WalkModule;

  class ApproachBall : public Option
  {
  public:
    ApproachBall(std::string const& id, std::shared_ptr<WalkModule> walkModule)
    : Option(id, "ApproachBall"),
      d_walkModule(walkModule)
    {
      d_turnScale          = Config::getSetting<double>("options.approach-ball.turn-speed-scale");
      d_maxForwardSpeed    = Config::getSetting<double>("options.approach-ball.max-forward-speed");
      d_minForwardSpeed    = Config::getSetting<double>("options.approach-ball.min-forward-speed");
      d_brakeDistance      = Config::getSetting<double>("options.approach-ball.brake-distance");
      d_lowerTurnLimitDegs = Config::getSetting<double>("options.approach-ball.lower-turn-limit-degs");
      d_upperTurnLimitDegs = Config::getSetting<double>("options.approach-ball.upper-turn-limit-degs");
    }

    virtual void reset() override;

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    void setStopDistance(double stopDistance)
    {
      d_useCustomStopDistance = true;
      d_stopDistance = stopDistance;
    }

  private:
    std::shared_ptr<WalkModule> d_walkModule;
    Setting<double>* d_turnScale;
    Setting<double>* d_maxForwardSpeed;
    Setting<double>* d_minForwardSpeed;
    Setting<double>* d_brakeDistance;
    Setting<double>* d_lowerTurnLimitDegs;
    Setting<double>* d_upperTurnLimitDegs;
    bool d_useCustomStopDistance;
    double d_stopDistance;
  };
}
