#pragma once

#include "../stateobject.hh"
#include "../../stats/average.hh"

#include <vector>
#include <Eigen/Geometry>

namespace bold
{
  class StationaryMapState : public StateObject
  {
  public:
    StationaryMapState(std::vector<Average<Eigen::Vector3d>> ballEstimates, std::vector<Average<Eigen::Vector3d>> goalEstimates)
    : d_ballEstimates(ballEstimates),
      d_goalEstimates(goalEstimates)
    {
      // Sort estimates such that those with greater numbers of observations appear first
      std::sort(d_ballEstimates.begin(), d_ballEstimates.end(), compareAverages);
      std::sort(d_goalEstimates.begin(), d_goalEstimates.end(), compareAverages);
    }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

    std::vector<Average<Eigen::Vector3d>> const& getBallEstimates() const { return d_ballEstimates; };
    std::vector<Average<Eigen::Vector3d>> const& getGoalEstimates() const { return d_goalEstimates; };

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

  private:
    static bool compareAverages(Average<Eigen::Vector3d> const& a, Average<Eigen::Vector3d> const& b)
    {
      return a.getCount() > b.getCount();
    }

    std::vector<Average<Eigen::Vector3d>> d_ballEstimates;
    std::vector<Average<Eigen::Vector3d>> d_goalEstimates;
  };
}
