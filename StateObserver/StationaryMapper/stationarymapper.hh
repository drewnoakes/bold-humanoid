#pragma once

#include "../typedstateobserver.hh"
#include "../../StateObject/StationaryMapState/stationarymapstate.hh"
#include "../../StateObject/WalkState/walkstate.hh"
#include "../../stats/average.hh"

#include <vector>
#include <Eigen/Core>

namespace bold
{
  class BehaviourControl;
  class Voice;

  class StationaryMapper : public StateObserver
  {
  public:
    StationaryMapper(std::shared_ptr <Voice> voice, std::shared_ptr <BehaviourControl> behaviourControl);

    void observe(SequentialTimer& timer) override;

  private:
    void updateStateObject() const;

    static void integrate(std::vector<Average<Eigen::Vector2d>>& estimates, Eigen::Vector2d pos, double mergeDistance);

    std::vector<Average<Eigen::Vector2d>> d_ballEstimates;
    std::vector<Average<Eigen::Vector2d>> d_goalEstimates;
    std::vector<Average<Eigen::Vector2d>> d_teammateEstimates;
    RadialOcclusionMap d_occlusionMap;
    bool d_hasData;
    std::shared_ptr<Voice> d_voice;
    std::shared_ptr<BehaviourControl> d_behaviourControl;
  };
}
