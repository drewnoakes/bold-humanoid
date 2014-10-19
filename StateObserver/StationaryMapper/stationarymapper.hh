#pragma once

#include "../typedstateobserver.hh"
#include "../../StateObject/StationaryMapState/stationarymapstate.hh"
#include "../../StateObject/WalkState/walkstate.hh"
#include "../../stats/average.hh"

#include <vector>
#include <Eigen/Core>

namespace bold
{
  class Voice;

  class StationaryMapper : public TypedStateObserver<WalkState>
  {
  public:
    StationaryMapper(std::shared_ptr<Voice> voice);

    virtual void observeTyped(std::shared_ptr<WalkState const> const& walkState, SequentialTimer& timer);

  private:
    void updateStateObject() const;

    static void integrate(std::vector<Average<Eigen::Vector2d>>& estimates, Eigen::Vector2d pos, double mergeDistance);

    std::vector<Average<Eigen::Vector2d>> d_ballEstimates;
    std::vector<Average<Eigen::Vector2d>> d_goalEstimates;
    std::vector<Average<Eigen::Vector2d>> d_teammateEstimates;
    RadialOcclusionMap d_occlusionMap;
    bool d_hasData;
    std::shared_ptr<Voice> d_voice;
  };
}
