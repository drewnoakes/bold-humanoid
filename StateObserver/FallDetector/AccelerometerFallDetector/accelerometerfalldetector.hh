#pragma once

#include "../falldetector.hh"

#include "../../typedstateobserver.hh"
#include "../../../StateObject/HardwareState/hardwarestate.hh"
#include "../../../stats/movingaverage.hh"

namespace bold
{
  class CM730Snapshot;

  class AccelerometerFallDetector : public FallDetector, public TypedStateObserver<HardwareState>
  {
  public:
    AccelerometerFallDetector(std::shared_ptr<Voice> voice);

    void observeTyped(std::shared_ptr<HardwareState const> const& hardwareState, SequentialTimer& timer) override;

  protected:
    void logFallData(std::stringstream& msg) const override;

  private:
    MovingAverage<int> d_xAvg;
    MovingAverage<int> d_yAvg;
    MovingAverage<int> d_zAvg;
  };
}
