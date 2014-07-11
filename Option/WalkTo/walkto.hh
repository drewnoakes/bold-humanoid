#pragma once

#import "../option.hh"
#include "../../MotionModule/WalkModule/walkmodule.hh"

#include <Eigen/Core>

namespace bold {
  
  class WalkTo : public Option
  {
  public:
    WalkTo(std::string const& id, std::shared_ptr<WalkModule> walkModule);

    OptionVector runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    void setTargetPosition(Eigen::Vector2d targetPos, double targetAngle)
    {
      d_targetPos = targetPos;
      d_targetAngle = targetAngle;
    }

  private:
    Eigen::Vector2d d_targetPos;
    double d_targetAngle;
    double d_turnDist;

    std::shared_ptr<WalkModule> d_walkModule;

    Setting<double>* d_turnScale;
    Setting<double>* d_maxForwardSpeed;
    Setting<double>* d_minForwardSpeed;
    Setting<double>* d_maxSidewaysSpeed;
    Setting<double>* d_minSidewaysSpeed;
    Setting<double>* d_brakeDistance;
    Setting<double>* d_lowerTurnLimitDegs;
    Setting<double>* d_upperTurnLimitDegs;

  };

  
}
