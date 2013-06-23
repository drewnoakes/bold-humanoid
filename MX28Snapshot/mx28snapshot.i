%{
#include <MX28Snapshot/mx28snapshot.hh>
%}

namespace bold
{
  class MX28Snapshot
  {
  public:
    uchar id;
    double presentPosition;
    double presentPositionValue;
    double presentSpeedRPM;
    double presentLoad;
    double presentVoltage;
    uchar presentTemp;
  };

  class StaticMX28State
  {
  public:
    unsigned short modelNumber;
    uchar firmwareVersion;
    uchar id;
    unsigned int baudBPS;
    unsigned int returnDelayTimeMicroSeconds;
    double angleLimitCW;
    double angleLimitCCW;
    uchar tempLimitHighCelcius;
    double voltageLimitLow;
    double voltageLimitHigh;
    unsigned short maxTorque;

    uchar statusRetLevel;

    //MX28Alarm alarmLed;
    //MX28Alarm alarmShutdown;

    bool torqueEnable;
    bool isEepromLocked;
  };
}
