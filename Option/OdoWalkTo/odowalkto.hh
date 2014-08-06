#pragma once

#include "../option.hh"

#include "../../MotionModule/WalkModule/walkmodule.hh"

#include <Eigen/Core>
#include <Eigen/Geometry>

namespace bold
{
  class OdoWalkTo : public Option
  {
  public:
    OdoWalkTo(std::string const& id, std::shared_ptr<WalkModule> walkModule);

    void setTarget(Eigen::Vector2d targetPos, Eigen::Vector2d targetFaceDir, double maxDist);

    double hasTerminated() override;

    OptionVector runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

  private:
    void updateProgress();

    std::shared_ptr<WalkModule> d_walkModule;
    Eigen::Vector2d d_targetFaceDir;

    double d_maxDist;
    Eigen::Affine3d d_lastOdoReading;
    Eigen::Affine3d d_progress;
  };
}
