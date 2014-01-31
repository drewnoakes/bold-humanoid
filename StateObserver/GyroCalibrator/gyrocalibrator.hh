#pragma once

#include "../typedstateobserver.hh"
#include "../../stats/movingaverage.hh"

namespace bold
{
  class HardwareState;
  class CM730Snapshot;

  enum class CalibrationState
  {
    /// Calibration is occurring
    PREPARING,
    /// Calibration failed to determine a reliable baseline
    ERRORED,
    /// Calibration completed successfully
    COMPLETE
  };

  class GyroCalibrator : public TypedStateObserver<HardwareState>
  {
  private:
    int d_windowSize;
    MovingAverage<int> d_fbAvgValue;
    MovingAverage<int> d_lrAvgValue;
    double d_standardDeviations;
    int d_fbCenter;
    int d_rlCenter;
    CalibrationState d_calibrationStatus;

  public:
    GyroCalibrator();

    void observeTyped(std::shared_ptr<HardwareState const> const& hardwareState, SequentialTimer& timer) override;

    void reset();

    CalibrationState getCalibrationStatus() const { return d_calibrationStatus; }
  };
}
