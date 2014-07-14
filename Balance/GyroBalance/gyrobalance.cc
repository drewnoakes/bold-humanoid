#include "gyrobalance.hh"

#include "../../Config/config.hh"
#include "../../State/state.hh"
#include "../../StateObject/BalanceState/balancestate.hh"
#include "../../StateObject/HardwareState/hardwarestate.hh"

using namespace bold;

GyroBalance::GyroBalance()
{
  d_kneeGain       = Config::getSetting<double>("walk-engine.balance.gyro.knee-gain");
  d_anklePitchGain = Config::getSetting<double>("walk-engine.balance.gyro.ankle-pitch-gain");
  d_hipRollGain    = Config::getSetting<double>("walk-engine.balance.gyro.hip-roll-gain");
  d_ankleRollGain  = Config::getSetting<double>("walk-engine.balance.gyro.ankle-roll-gain");
}

BalanceOffset GyroBalance::computeCorrection(double targetPitchRads) const
{
  auto hw = State::get<HardwareState>();

  ASSERT(hw);

  auto gyroRaw = hw->getCM730State().getBalancedGyroValue();

  int rlErr = -gyroRaw.y();
  int fbErr = gyroRaw.x();

  auto correction = BalanceOffset();

  correction.hipRollR = static_cast<float>(-rlErr * d_hipRollGain->getValue());
  correction.hipRollL = static_cast<float>(-rlErr * d_hipRollGain->getValue());

  correction.kneeR = static_cast<float>(-fbErr * d_kneeGain->getValue());
  correction.kneeL = static_cast<float>(fbErr * d_kneeGain->getValue());

  correction.anklePitchR = static_cast<float>(fbErr * d_anklePitchGain->getValue());
  correction.anklePitchL = static_cast<float>(-fbErr * d_anklePitchGain->getValue());

  correction.ankleRollR = static_cast<float>(-rlErr * d_ankleRollGain->getValue());
  correction.ankleRollL = static_cast<float>(-rlErr * d_ankleRollGain->getValue());

  State::make<BalanceState>(correction);

  return correction;
}
