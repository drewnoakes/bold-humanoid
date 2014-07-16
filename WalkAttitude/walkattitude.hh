#pragma once

#include "../Smoother/LinearSmoother/linearsmoother.hh"

namespace bold
{
  typedef unsigned char uchar;

  template<typename> class Setting;
  enum class WalkStatus : uchar;

  /** Calculates torso attitude during walking to maintain stability. */
  class WalkAttitude
  {
  public:
    WalkAttitude();

    void update(WalkStatus walkStatus, LinearSmoother const& xAmp, LinearSmoother const& yAmp, LinearSmoother const& turnAmp);

    void reset();

    double getHipPitch() const { return d_hipPitch; }

  private:
    double d_hipPitch;

    // hip pitch calculation parameters
    Setting<double>* d_stableHipPitch;
    Setting<double>* d_minHipPitch;
    Setting<double>* d_maxHipPitch;
    Setting<double>* d_maxHipPitchAtSpeed;
    Setting<double>* d_fwdAccelerationHipPitchFactor;
    Setting<double>* d_bwdAccelerationHipPitchFactor;
  };
}
