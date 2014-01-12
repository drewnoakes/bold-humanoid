#pragma once

#include "../stateobserver.hh"
#include "../../Clock/clock.hh"
#include "../../StateObject/HardwareState/hardwarestate.hh"
#include "../../stats/movingaverage.hh"
#include "../../util/schmitttrigger.hh"
#include "../../Voice/voice.hh"

namespace bold
{
  class HealthAndSafety : public TypedStateObserver<HardwareState>
  {
  public:
    HealthAndSafety(std::shared_ptr<Voice> voice);

    void observeTyped(std::shared_ptr<HardwareState const> state) override;

  private:
    std::shared_ptr<Voice> d_voice;
    MovingAverage<int> d_voltageMovingAverage;
    SchmittTrigger<float> d_voltageTrigger;
    int d_temperatureThreshold;
    Clock::Timestamp d_lastVoltageWarningTime;
    Clock::Timestamp d_lastTemperatureWarningTime;
  };
}
