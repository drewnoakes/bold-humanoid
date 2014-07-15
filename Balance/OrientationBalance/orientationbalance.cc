#include "orientationbalance.hh"

#include "../../Config/config.hh"
#include "../../State/state.hh"
#include "../../StateObject/OrientationState/orientationstate.hh"

using namespace bold;

OrientationBalance::OrientationBalance()
{
  d_kneeGain       = Config::getSetting<double>("balance.orientation.knee-gain");
  d_anklePitchGain = Config::getSetting<double>("balance.orientation.ankle-pitch-gain");
  d_hipRollGain    = Config::getSetting<double>("balance.orientation.hip-roll-gain");
  d_ankleRollGain  = Config::getSetting<double>("balance.orientation.ankle-roll-gain");
}

BalanceOffset OrientationBalance::computeCorrection(double targetPitchRads) const
{
  auto orientation = State::get<OrientationState>();

  ASSERT(orientation);

  double rollError = -orientation->getRollAngle();
  double pitchError = -targetPitchRads - orientation->getPitchAngle();

  std::cout << "targetPitch=" << (int)Math::radToDeg(targetPitchRads)
            << " currPitch="  << (int)Math::radToDeg(-orientation->getPitchAngle())
            << " pitchError=" << (int)Math::radToDeg(pitchError)
            << " rollErr="    << (int)Math::radToDeg(rollError)
            << std::endl;

  auto correction = BalanceOffset();

  correction.hipRoll    = static_cast<short>(round(-rollError * d_hipRollGain->getValue()));
  correction.knee       = static_cast<short>(round(-pitchError * d_kneeGain->getValue()));
  correction.anklePitch = static_cast<short>(round( pitchError * d_anklePitchGain->getValue()));
  correction.ankleRoll  = static_cast<short>(round(-rollError * d_ankleRollGain->getValue()));

  return correction;
}
