#include "walkattitude.hh"

#include "../../Config/config.hh"
#include "../MotionModule/WalkModule/walkmodule.hh"

using namespace bold;

WalkAttitude::WalkAttitude()
: d_stableHipPitch(Config::getSetting<double>("walk-module.hip-pitch.stable-angle")),
  d_minHipPitch(Config::getSetting<double>("walk-module.hip-pitch.min-angle")),
  d_maxHipPitch(Config::getSetting<double>("walk-module.hip-pitch.max-angle")),
  d_maxHipPitchAtSpeed(Config::getSetting<double>("walk-module.hip-pitch.max-at-fwd-speed")),
  d_fwdAccelerationHipPitchFactor(Config::getSetting<double>("walk-module.hip-pitch.fwd-acc-factor")),
  d_bwdAccelerationHipPitchFactor(Config::getSetting<double>("walk-module.hip-pitch.bwd-acc-factor"))
{}

void WalkAttitude::update(WalkStatus walkStatus, LinearSmoother const& xAmp, LinearSmoother const& yAmp, LinearSmoother const& turnAmp)
{
  if (walkStatus == WalkStatus::Stabilising)
  {
    d_hipPitch = d_stableHipPitch->getValue();
    return;
  }

  // TODO verify this works for walking backwards (-ve x)
  // TODO consider turn speed and sideways speed

  // Determine a ratio [0,1] that determines how far
  double alpha = xAmp.getCurrent() / d_maxHipPitchAtSpeed->getValue();

//    // Estimate future forward acceleration by comparing the target forward speed with the current.
//    // Note that the target can fluctuate considerably, so this value may be quite noisy.
//    double xAcc = d_xAmpSmoother.getTarget() - xAmp;

  // The change in xAmp gives a direction and magnitude of our acceleration in the forward direction.
  // TODO actually the last delta will either be zero or +/- the step size!!
  double xAcc = xAmp.getLastDelta();

  if (xAcc > 0)
    alpha += d_fwdAccelerationHipPitchFactor->getValue() * xAcc;
  else if (xAcc < 0)
    alpha += d_bwdAccelerationHipPitchFactor->getValue() * xAcc;

  d_hipPitch = Math::lerp(
    Math::clamp(alpha, 0.0, 1.0),
    d_minHipPitch->getValue(),
    d_maxHipPitch->getValue());
}

void WalkAttitude::reset()
{
  d_hipPitch = d_stableHipPitch->getValue();
}
