#include "orientationbalance.hh"

#include "../../Config/config.hh"
#include "../../State/state.hh"
#include "../../StateObject/BalanceState/balancestate.hh"
#include "../../StateObject/OrientationState/orientationstate.hh"

using namespace bold;

OrientationBalance::OrientationBalance()
{
  d_kneeGain       = Config::getSetting<double>("walk-engine.balance.orientation.knee-gain");
  d_anklePitchGain = Config::getSetting<double>("walk-engine.balance.orientation.ankle-pitch-gain");
  d_hipRollGain    = Config::getSetting<double>("walk-engine.balance.orientation.hip-roll-gain");
  d_ankleRollGain  = Config::getSetting<double>("walk-engine.balance.orientation.ankle-roll-gain");
}

BalanceOffset OrientationBalance::computeCorrection(double targetPitchRads) const
{
  auto orientation = State::get<OrientationState>();

  ASSERT(orientation);

  double rollError = -orientation->getRollAngle();
  double pitchError = targetPitchRads - orientation->getPitchAngle();

  auto correction = BalanceOffset();

  correction.hipRollR    = static_cast<short>(round(-rollError * d_hipRollGain->getValue()));
  correction.hipRollL    = static_cast<short>(round(-rollError * d_hipRollGain->getValue()));

  correction.kneeR       = static_cast<short>(round(-pitchError * d_kneeGain->getValue()));
  correction.kneeL       = static_cast<short>(round( pitchError * d_kneeGain->getValue()));

  correction.anklePitchR = static_cast<short>(round( pitchError * d_anklePitchGain->getValue()));
  correction.anklePitchL = static_cast<short>(round(-pitchError * d_anklePitchGain->getValue()));

  correction.ankleRollR  = static_cast<short>(round(-rollError * d_ankleRollGain->getValue()));
  correction.ankleRollL  = static_cast<short>(round(-rollError * d_ankleRollGain->getValue()));

  State::make<BalanceState>(correction);

  return correction;
}
