#include "gyrobalance.hh"

#include "../../Config/config.hh"
#include "../../State/state.hh"
#include "../../StateObject/HardwareState/hardwarestate.hh"

using namespace bold;

GyroBalance::GyroBalance()
{
  d_kneeGain       = Config::getSetting<double>("balance.gyro.knee-gain");
  d_anklePitchGain = Config::getSetting<double>("balance.gyro.ankle-pitch-gain");
  d_hipRollGain    = Config::getSetting<double>("balance.gyro.hip-roll-gain");
  d_ankleRollGain  = Config::getSetting<double>("balance.gyro.ankle-roll-gain");
}

BalanceOffset GyroBalance::computeCorrection(double targetPitchRads) const
{
  auto hw = State::get<HardwareState>();

  ASSERT(hw);

  auto gyroRaw = hw->getCM730State().getBalancedGyroValue();

  int rlErr = -gyroRaw.y();
  int fbErr = gyroRaw.x();

  auto correction = BalanceOffset();

  correction.hipRoll    = static_cast<short>(round(-rlErr * d_hipRollGain->getValue()));
  correction.knee       = static_cast<short>(round(-fbErr * d_kneeGain->getValue()));
  correction.anklePitch = static_cast<short>(round( fbErr * d_anklePitchGain->getValue()));
  correction.ankleRoll  = static_cast<short>(round(-rlErr * d_ankleRollGain->getValue()));

  return correction;
}
