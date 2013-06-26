#pragma once

#include "../../Control/control.hh"
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
    {
      d_controls.push_back(Control::createInt("Lower turn limit (degs)", [this](){ return Math::radToDeg(d_lowerTurnLimitRads); }, [this](int value) { d_lowerTurnLimitRads = Math::degToRad(value); }));
      d_controls.push_back(Control::createInt("Upper turn limit (degs)", [this](){ return Math::radToDeg(d_upperTurnLimitRads); }, [this](int value) { d_upperTurnLimitRads = Math::degToRad(value); }));
      d_controls.push_back(Control::createInt("Min forward speed", [this](){ return d_minForwardSpeed; }, [this](int value) { d_minForwardSpeed = value; }));
      d_controls.push_back(Control::createInt("Max forward speed", [this](){ return d_maxForwardSpeed; }, [this](int value) { d_maxForwardSpeed = value; }));
    }

    std::vector<std::shared_ptr<Option>> runPolicy() override;

    std::vector<std::shared_ptr<Control const>> getControls() const { return d_controls; }

  private:
    std::vector<std::shared_ptr<Control const>> d_controls;
    std::shared_ptr<Ambulator> d_ambulator;
    double d_turnScale;
    double d_maxForwardSpeed;
    double d_minForwardSpeed;
    double d_breakDist;
    double d_lowerTurnLimitRads;
    double d_upperTurnLimitRads;
  };
}
