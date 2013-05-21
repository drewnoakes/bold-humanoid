#pragma once

#include "../../MovingAverage/movingaverage.hh"
#include "../../StateObject/HardwareState/hardwarestate.hh"
#include "../../StateObserver/stateobserver.hh"

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

    void observeTyped(std::shared_ptr<HardwareState const> hardwareState) override;

    FallState getFallenState() const { return d_fallenState; }

  private:
    int d_windowSize;
    MovingAverage<int> d_fbAvgValue;
    int d_forwardLimitValue;
    int d_backwardLimitValue;
    FallState d_fallenState;
  };
}