#ifndef BOLD_DEBUGGER_HH
#define BOLD_DEBUGGER_HH

#define RED_LED 0x01;

namespace bold
{
  class Debugger
  {
  private:
    bool d_isBallObserved;
    int d_lastLEDValue;

    void updateLEDs(Robot::CM730& cm730)
    {
      int value = 0;
      if (d_isBallObserved)
        value |= RED_LED;
      if (value != d_lastLEDValue)
      {
        cm730.WriteByte(Robot::CM730::P_LED_PANNEL, value, NULL);
        d_lastLEDValue = value;
      }
    }

  public:
    Debugger()
    : d_isBallObserved(false),
      d_lastLEDValue(0xff)
    {}

    void setIsBallObserved(Robot::CM730& cm730, bool isBallObserved)
    {
      d_isBallObserved = isBallObserved;
      updateLEDs(cm730);
    }
  };
}

#endif
