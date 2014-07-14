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
    short hipRollR;
    short hipRollL;
    short kneeR;
    short kneeL;
    short anklePitchR;
    short anklePitchL;
    short ankleRollR;
    short ankleRollL;
  };

  class Balance
  {
  public:
    virtual BalanceOffset computeCorrection(double targetPitchRads) const = 0;
  };
}
