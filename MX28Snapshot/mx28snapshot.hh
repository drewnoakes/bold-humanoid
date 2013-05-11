#pragma once

#include "../MX28Alarm/mx28alarm.hh"

namespace bold
{
  typedef unsigned char uchar;

  class BulkReadTable;

  // http://support.robotis.com/en/techsupport_eng.htm#product/dynamixel/mx_series/mx-28.htm

  // TODO rename as MX28State

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

    MX28Snapshot(BulkReadTable const& data, int const mx28ID);
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

    /**
    * Controls when a status packet is returned.
    *
    * 0 - only for PING command
    * 1 - only for READ command
    * 2 - for all commands
    *
    * Never returned if instruction is a broadcast packet.
    */
    uchar statusRetLevel;

    /// Specifies conditions for the alarm LED to be turned on
    MX28Alarm alarmLed;
    /// Specifies conditions for the device to shut down
    MX28Alarm alarmShutdown;

    // RAM AREA

    bool torqueEnable;
    bool led;
    double gainP;
    double gainI;
    double gainD;
    double goalPositionRads;
    double movingSpeedRPM;
    double torqueLimit;
    bool isInstructionRegistered;
    bool isMoving;
    bool isEepromLocked;
//  uchar punch; // apparently this value is unused
//  double goalAcceleration; // TODO introduce this from the read

    StaticMX28State(BulkReadTable const& data, int const mx28ID);
  };
}
