#pragma once

#include "../typedstateobserver.hh"
#include "../../Clock/clock.hh"
#include "../../stats/movingaverage.hh"
#include "../../util/schmitttrigger.hh"
#include "../../Setting/setting.hh"
#include "../../JointId/jointid.hh"

#include <array>
#include <vector>

namespace bold
{
  class Voice;
  class HardwareState;

  class HealthAndSafety : public TypedStateObserver<HardwareState>
  {
  public:
    HealthAndSafety(std::shared_ptr<Voice> voice);

    void observeTyped(std::shared_ptr<HardwareState const> const& state, SequentialTimer& timer) override;

  private:
    void processVoltage(std::shared_ptr<HardwareState const> const& state);
    void processTemperature(std::shared_ptr<HardwareState const> const& state);

    std::shared_ptr<Voice> d_voice;

    MovingAverage<float> d_voltageMovingAverage;
    SchmittTrigger<float> d_voltageTrigger;
    Clock::Timestamp d_lastVoltageWarningTime;

    std::vector<MovingAverage<float>> d_averageTempByJoint;
    std::array<uchar,(int)JointId::MAX+1> d_lastTempByJoint;
    Setting<int>* d_temperatureThreshold;
  };
}
