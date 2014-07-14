#pragma once

#include "../balance.hh"

namespace bold
{
  template<typename> class Setting;

  class OrientationBalance : public Balance
  {
  public:
    OrientationBalance();

    virtual BalanceOffset computeCorrection(double targetPitchRads) const override;

  private:
    Setting<double>* d_kneeGain;
    Setting<double>* d_hipRollGain;
    Setting<double>* d_anklePitchGain;
    Setting<double>* d_ankleRollGain;
  };
}
