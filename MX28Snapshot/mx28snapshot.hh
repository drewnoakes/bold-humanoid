#ifndef BOLD_MX28_SNAPSHOT_HH
#define BOLD_MX28_SNAPSHOT_HH

#include "../robotis/Framework/include/CM730.h"
#include "../MX28Alarm/mx28alarm.hh"

#include <iostream>
#include <cassert>

namespace bold
{
  class MX28Snapshot
  {
  public:

    typedef unsigned char uchar;

    // EEPROM AREA

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

    MX28Alarm alarmLed;
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
    double presentPosition;
    double presentSpeedRPM;
    double presentLoad;
    double presentVoltage;
    uchar presentTemp;
    bool isInstructionRegistered;
    bool isMoving;
    bool isEepromLocked;

    // apparently this value is unused
//     uchar punch;

    MX28Snapshot() {}

    MX28Snapshot(Robot::BulkReadData& cm730, int const mx28ID);

  private:

    static unsigned short readTableWord(uchar* table, int addr);
    static double angleValueToRads(unsigned int value);
    static double valueToRPM(unsigned int value);
  };
}

#endif
