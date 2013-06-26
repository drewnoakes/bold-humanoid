#pragma once

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
        d_ambulator(ambulator),
        d_turnScale(getParam("turnScale", 17.5)),
        d_maxForwardSpeed(getParam("maxForwardSpeed", 30.0)),
        d_minForwardSpeed(getParam("minForwardSpeed", 5.0)),
        d_breakDist(getParam("breakDist", 0.5)),
        d_lowerTurnLimitRads(getParam("lowerTurnLimitDegs", Math::degToRad(10))),
        d_upperTurnLimitRads(getParam("upperTurnLimitDegs", Math::degToRad(30)))
    {}

    std::vector<std::shared_ptr<Option>> runPolicy() override;

  private:
    std::shared_ptr<Ambulator> d_ambulator;
    double d_turnScale;
    double d_maxForwardSpeed;
    double d_minForwardSpeed;
    double d_breakDist;
    double d_lowerTurnLimitRads;
    double d_upperTurnLimitRads;
  };
}
