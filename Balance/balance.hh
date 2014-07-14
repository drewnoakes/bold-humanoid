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
    short hipRoll;
    short knee;
    short anklePitch;
    short ankleRoll;
  };

  class Balance
  {
  public:
    virtual BalanceOffset computeCorrection(double targetPitchRads) const = 0;
  };
}
