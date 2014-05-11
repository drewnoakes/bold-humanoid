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
    FORWARD,
    LEFT,
    RIGHT
  };

  std::string getFallStateName(FallState fallState);

  class FallDetector : public TypedStateObserver<HardwareState>
  {
  public:
    FallDetector(std::shared_ptr<Voice> voice);

    void observeTyped(std::shared_ptr<HardwareState const> const& hardwareState, SequentialTimer& timer) override;

    FallState getFallenState() const { return d_fallenState; }

  private:
    std::shared_ptr<Voice> d_voice;
    MovingAverage<int> d_fbAvgValue;
    MovingAverage<int> d_lrAvgValue;
    Setting<int>* d_maxLimitValue;
    Setting<double>* d_turnFbRatio;
    FallState d_fallenState;
    Clock::Timestamp d_startTime;
    int d_neutralAccPos = 512;
  };
}
