#pragma once

#include "../typedstateobserver.hh"
#include "../../StateObject/BodyState/bodystate.hh"
#include "../../stats/movingaverage.hh"
#include "../../util/schmitttrigger.hh"

namespace bold
{
  template<typename> class Setting;
  class Voice;

  class JamDetector : public TypedStateObserver<BodyState>
  {
  public:
    JamDetector(std::shared_ptr<Voice> voice);

    void observeTyped(std::shared_ptr<BodyState const> const& bodyState, SequentialTimer& timer) override;

  private:
    struct JointErrorTracker
    {
      JointErrorTracker(int lowThreshold, int highThreshold, int windowSize)
      : trigger(lowThreshold, highThreshold, false),
        movingAverage(windowSize)
      {}

      SchmittTrigger<int> trigger;
      MovingAverage<int> movingAverage;
    };

    std::vector<JointErrorTracker> d_trackerByJointId;
    std::shared_ptr<Voice> d_voice;
    Setting<bool>* d_enabled;
  };
}
