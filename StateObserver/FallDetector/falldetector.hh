#pragma once

#include "../typedstateobserver.hh"
#include "../../StateObject/HardwareState/hardwarestate.hh"
#include "../../stats/movingaverage.hh"

namespace bold
{
  class CM730Snapshot;
  template<typename> class Setting;
  class Voice;

  enum class FallState
  {
    STANDUP,
    BACKWARD,
    FORWARD
  };

  class FallDetector : public TypedStateObserver<HardwareState>
  {
  public:
    FallDetector(std::shared_ptr<Voice> voice);

    void observeTyped(std::shared_ptr<HardwareState const> const& hardwareState, SequentialTimer& timer) override;

    FallState getFallenState() const { return d_fallenState; }

  private:
    std::shared_ptr<Voice> d_voice;
    MovingAverage<int> d_fbAvgValue;
    Setting<int>* d_forwardLimitValue;
    Setting<int>* d_backwardLimitValue;
    FallState d_fallenState;
  };
}
