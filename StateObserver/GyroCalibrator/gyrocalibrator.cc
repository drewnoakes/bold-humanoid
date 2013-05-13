#include "gyrocalibrator.hh"

#include "../CM730Snapshot/cm730snapshot.hh"
#include "../StateObject/HardwareState/hardwarestate.hh"

#include <cmath>
#include <iostream>

using namespace bold;
using namespace std;

GyroCalibrator::GyroCalibrator()
: d_windowSize(100),
  d_fbAvgValue(d_windowSize),
  d_lrAvgValue(d_windowSize),
  d_standardDeviations(2.0)
{
  reset();
}

void GyroCalibrator::reset()
{
  d_calibrationStatus = CalibrationState::PREPARING;
  d_fbCenter = -1;
  d_rlCenter = -1;
}

void GyroCalibrator::observeTyped(std::shared_ptr<HardwareState const> hardwareState)
{
  if (d_calibrationStatus == CalibrationState::COMPLETE)
    return;

  cout << "[GyroCalibrator::observeTyped] observing" << endl;
  
  // TODO why not do up/down (Z) ?
  // TODO do the axes of the acc/gyro match the torso axes we've chosen?

  auto const& accRaw = hardwareState->getCM730State()->accRaw;

  d_fbAvgValue.next(accRaw.y());
  d_lrAvgValue.next(accRaw.x());

  if (d_fbAvgValue.count() != d_windowSize)
    return;

  double fbStdDev = d_fbAvgValue.calculateStdDev();
  double lrStdDev = d_lrAvgValue.calculateStdDev();

  if (fbStdDev < d_standardDeviations && lrStdDev < d_standardDeviations)
  {
    d_fbCenter = (int)round(d_fbAvgValue.getAverage());
    d_rlCenter = (int)round(d_lrAvgValue.getAverage());
    d_calibrationStatus = CalibrationState::COMPLETE;

    cout << "[GyroCalibrator::update] Gyro calibrated with center at " << d_fbCenter << " (f/b) and " << d_rlCenter << " (l/r)" << endl;
  }
  else
  {
    // Reset. We might have been moving and we want to ensure that no
    // samples in the window are considered when calculating the true
    // zero point.
    reset();

    d_calibrationStatus = CalibrationState::ERRORED;
  }
}
