#pragma once

#include "../option.hh"

#include "../../MotionModule/WalkModule/walkmodule.hh"

#include <Eigen/Core>
#include <Eigen/Geometry>

namespace bold {
  
  class OdoWalkTo : public Option
  {
  public:
    OdoWalkTo(std::string const& id, std::shared_ptr<WalkModule> walkModule);

    void setTargetPos(Eigen::Vector3d targetPos, double maxDist);

    double hasTerminated() override;

    OptionVector runPolicy() override;

  private:
    std::shared_ptr<WalkModule> d_walkModule;

    Eigen::Vector3d d_targetPos;
    double d_maxDist;
    Eigen::Affine3d d_lastOdoReading;
    Eigen::Affine3d d_progress;

    void updateProgress();
  };

  
}
