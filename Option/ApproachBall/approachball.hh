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
        d_turnScale(getParam("turnScale", 25.0)),
        d_maxForwardSpeed(getParam("maxForwardSpeed", 40.0)),
        d_minForwardSpeed(getParam("minForwardSpeed", 5.0)),
        d_brakeDistance(getParam("breakDist", 0.45)),
        d_lowerTurnLimitRads(getParam("lowerTurnLimitDegs", Math::degToRad(20))),
        d_upperTurnLimitRads(getParam("upperTurnLimitDegs", Math::degToRad(35)))
    {
      d_controls.push_back(Control::createInt("Lower turn limit (degs)", [this](){ return Math::radToDeg(d_lowerTurnLimitRads); }, [this](int value) { d_lowerTurnLimitRads = Math::degToRad(value); }));
      d_controls.push_back(Control::createInt("Upper turn limit (degs)", [this](){ return Math::radToDeg(d_upperTurnLimitRads); }, [this](int value) { d_upperTurnLimitRads = Math::degToRad(value); }));
      d_controls.push_back(Control::createInt("Min forward speed",   [this](){ return d_minForwardSpeed; }, [this](int value) { d_minForwardSpeed = value; }));
      d_controls.push_back(Control::createInt("Max forward speed",   [this](){ return d_maxForwardSpeed; }, [this](int value) { d_maxForwardSpeed = value; }));
      d_controls.push_back(Control::createInt("Turn speed scale",    [this](){ return d_turnScale; }, [this](int value) { d_turnScale = value; }));
      d_controls.push_back(Control::createInt("Break distance (cm)", [this](){ return d_brakeDistance * 100; }, [this](int value) { d_brakeDistance = value/100.0; }));
    }

    std::vector<std::shared_ptr<Option>> runPolicy() override;

    std::vector<std::shared_ptr<Control const>> getControls() const { return d_controls; }

  private:
    std::vector<std::shared_ptr<Control const>> d_controls;
    std::shared_ptr<Ambulator> d_ambulator;
    double d_turnScale;
    double d_maxForwardSpeed;
    double d_minForwardSpeed;
    double d_brakeDistance;
    double d_lowerTurnLimitRads;
    double d_upperTurnLimitRads;
  };
}
