#pragma once

#include "../option.hh"
#include "../../StateObject/StationaryMapState/stationarymapstate.hh"
#include "../../stats/average.hh"

#include <vector>
#include <Eigen/Core>

namespace bold
{
  class BuildStationaryMap : public Option
  {
  public:
    BuildStationaryMap(std::string const& id);

    virtual std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    virtual void reset() override;

  private:
    void updateStateObject() const;

    static void integrate(std::vector<Average<Eigen::Vector3d>>& estimates, Eigen::Vector3d pos, double mergeDistance);

    std::vector<Average<Eigen::Vector3d>> d_ballEstimates;
    std::vector<Average<Eigen::Vector3d>> d_goalEstimates;
    std::vector<Average<Eigen::Vector3d>> d_teammateEstimates;
    RadialOcclusionMap d_occlusionMap;
  };
}
