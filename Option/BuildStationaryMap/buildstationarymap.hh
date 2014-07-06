#pragma once

#include "../option.hh"
#include "../../StateObject/StationaryMapState/stationarymapstate.hh"
#include "../../stats/average.hh"

#include <vector>
#include <Eigen/Core>

namespace bold
{
  class Voice;

  class BuildStationaryMap : public Option
  {
  public:
    BuildStationaryMap(std::string const& id, std::shared_ptr<Voice> voice);

    virtual std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    virtual void reset() override;

  private:
    void updateStateObject() const;

    static void integrate(std::vector<Average<Eigen::Vector2d>>& estimates, Eigen::Vector2d pos, double mergeDistance);

    std::shared_ptr<Voice> d_voice;
    std::vector<Average<Eigen::Vector2d>> d_ballEstimates;
    std::vector<Average<Eigen::Vector2d>> d_goalEstimates;
    std::vector<Average<Eigen::Vector2d>> d_teammateEstimates;
    RadialOcclusionMap d_occlusionMap;
  };
}
