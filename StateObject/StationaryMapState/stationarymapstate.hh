#pragma once

#include "../stateobject.hh"
#include "../../stats/average.hh"

#include <Eigen/Geometry>

namespace bold
{
  class StationaryMapState : public StateObject
  {
  public:
    StationaryMapState(std::vector<Average<Eigen::Vector3d>> ballEstimates, std::vector<Average<Eigen::Vector3d>> goalEstimates)
    : d_ballEstimates(ballEstimates),
      d_goalEstimates(goalEstimates)
    {}

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

  private:
    std::vector<Average<Eigen::Vector3d>> d_ballEstimates;
    std::vector<Average<Eigen::Vector3d>> d_goalEstimates;
  };
}
