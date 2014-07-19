#include "walkattitude.hh"

#include "../../Config/config.hh"
#include "../MotionModule/WalkModule/walkmodule.hh"

using namespace bold;

WalkAttitude::WalkAttitude()
: d_stableHipPitch(Config::getSetting<double>("walk-module.hip-pitch.stable-angle")),
  d_maxFwdHipPitch(Config::getSetting<double>("walk-module.hip-pitch.max-fwd-angle")),
  d_maxBwdHipPitch(Config::getSetting<double>("walk-module.hip-pitch.max-bwd-angle")),
  d_maxHipPitchAtFwdSpeed(Config::getSetting<double>("walk-module.hip-pitch.max-at-fwd-speed")),
  d_maxHipPitchAtBwdSpeed(Config::getSetting<double>("walk-module.hip-pitch.max-at-bwd-speed")),
  d_fwdAccHipPitchDelta(Config::getSetting<double>("walk-module.hip-pitch.fwd-acc-delta")),
  d_bwdAccHipPitchDelta(Config::getSetting<double>("walk-module.hip-pitch.bwd-acc-delta"))
{}

void WalkAttitude::update(WalkStatus walkStatus, LinearSmoother const& xAmp, LinearSmoother const& yAmp, LinearSmoother const& turnAmp)
{
  if (walkStatus == WalkStatus::Stabilising)
  {
    d_hipPitch = d_stableHipPitch->getValue();
    return;
  }

  // TODO consider turn speed and sideways speed

  double xCurrent = xAmp.getCurrent();

  if (xCurrent == 0)
  {
    // Not moving forwards or backwards
    d_hipPitch = d_stableHipPitch->getValue();
    return;
  }

  double alpha, max;
  if (xCurrent > 0)
  {
    // Moving forwards
    alpha = xCurrent / d_maxHipPitchAtFwdSpeed->getValue();
    max = d_maxFwdHipPitch->getValue();
  }
  else
  {
    // Moving backwards
    alpha = fabs(xCurrent) / d_maxHipPitchAtBwdSpeed->getValue();
    max = d_maxBwdHipPitch->getValue();
  }

  ASSERT(alpha >= 0);
  double hipPitch = Math::lerp(
    Math::clamp(alpha, 0.0, 1.0),
    d_stableHipPitch->getValue(),
    max);

//    // Estimate future forward acceleration by comparing the target forward speed with the current.
//    // Note that the target can fluctuate considerably, so this value may be quite noisy.
//    double xAcc = d_xAmpSmoother.getTarget() - xAmp;

  // The change in xAmp gives a direction and magnitude of our acceleration in the forward direction.
  // TODO actually the last delta will either be zero or +/- the step size!!
  double xAcc = xAmp.getLastDelta();

  if (xAcc > 0)
    hipPitch += d_fwdAccHipPitchDelta->getValue();
  else if (xAcc < 0)
    hipPitch += d_bwdAccHipPitchDelta->getValue();

  d_hipPitch = hipPitch;
}

void WalkAttitude::reset()
{
  d_hipPitch = d_stableHipPitch->getValue();
}
