#pragma once

#include "../option.hh"

#include "../../minIni/minIni.h"

namespace bold
{
  class Ambulator;

  class ApproachBall : public Option
  {
  public:
    ApproachBall(minIni const& ini, std::string const& id, std::shared_ptr<Ambulator> ambulator)
      : Option(id),
        d_ambulator(ambulator),
        d_turnScale(ini.getd("Approach Ball", "TurnScale", 17.5)),
        d_maxForwardSpeed(ini.getd("Approach Ball", "MaxForwardSpeed", 30.0)),
        d_minForwardSpeed(ini.getd("Approach Ball", "MinForwardSpeed", 5.0)),
        d_breakDist(ini.getd("Approach Ball", "BreakDist", 0.5))
    {}

    OptionList runPolicy() override;

  private:
    std::shared_ptr<Ambulator> d_ambulator;
    double d_turnScale;
    double d_maxForwardSpeed;
    double d_minForwardSpeed;
    double d_breakDist;
  };
}
