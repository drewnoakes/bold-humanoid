#pragma once

#include "../../Config/config.hh"
#include "../../Math/math.hh"
#include "../option.hh"

namespace bold
{
  class Ambulator;

  class ApproachBall : public Option
  {
  public:
    ApproachBall(std::string const& id, std::shared_ptr<Ambulator> ambulator)
    : Option(id),
      d_ambulator(ambulator)
    {
      d_turnScale          = Config::getSetting<double>("options.approach-ball.turn-speed-scale");
      d_maxForwardSpeed    = Config::getSetting<double>("options.approach-ball.max-forward-speed");
      d_minForwardSpeed    = Config::getSetting<double>("options.approach-ball.min-forward-speed");
      d_brakeDistance      = Config::getSetting<double>("options.approach-ball.brake-distance");
      d_lowerTurnLimitDegs = Config::getSetting<double>("options.approach-ball.lower-turn-limit-degs");
      d_upperTurnLimitDegs = Config::getSetting<double>("options.approach-ball.upper-turn-limit-degs");
    }

    std::vector<std::shared_ptr<Option>> runPolicy() override;

  private:
    std::shared_ptr<Ambulator> d_ambulator;
    Setting<double>* d_turnScale;
    Setting<double>* d_maxForwardSpeed;
    Setting<double>* d_minForwardSpeed;
    Setting<double>* d_brakeDistance;
    Setting<double>* d_lowerTurnLimitDegs;
    Setting<double>* d_upperTurnLimitDegs;
  };
}
