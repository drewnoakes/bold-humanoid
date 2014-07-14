#pragma once

namespace bold
{
  enum class BalanceMode
  {
    None = 0,
    Gyro = 1,
    Orientation = 2
  };

  struct BalanceOffset
  {
    float hipRollR;
    float hipRollL;
    float kneeR;
    float kneeL;
    float anklePitchR;
    float anklePitchL;
    float ankleRollR;
    float ankleRollL;
  };

  class Balance
  {
  public:
    virtual BalanceOffset computeCorrection(double targetPitchRads) const = 0;
  };
}
