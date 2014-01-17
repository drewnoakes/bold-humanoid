#pragma once

#include "../typedstateobserver.hh"
#include "../../StateObject/HardwareState/hardwarestate.hh"
#include "../../stats/movingaverage.hh"

namespace bold
{
  class CM730Snapshot;

  enum class FallState
  {
    STANDUP,
    BACKWARD,
    FORWARD
  };

  class FallDetector : public TypedStateObserver<HardwareState>
  {
  public:
    FallDetector();

    void observeTyped(std::shared_ptr<HardwareState const> hardwareState, SequentialTimer& timer) override;

    FallState getFallenState() const { return d_fallenState; }

  private:
    int d_windowSize;
    MovingAverage<int> d_fbAvgValue;
    int d_forwardLimitValue;
    int d_backwardLimitValue;
    FallState d_fallenState;
  };
}
