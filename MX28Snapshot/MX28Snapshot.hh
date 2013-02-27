#ifndef BOLD_MX28_SNAPSHOT_HH
#define BOLD_MX28_SNAPSHOT_HH

#include "../robotis/Framework/include/CM730.h"

using namespace Robot;

class MX28Snapshot
{
private:
  static unsigned short readTableWord(unsigned char* table, int addr);
  static double angleValueToRads(unsigned int value);
  static double valueToRPM(unsigned int value);

public:

  // EEPROM AREA

  unsigned short modelNumber;
  unsigned char firmwareVersion;
  unsigned char id;
  unsigned int baudBPS;
  unsigned int returnDelayTimeMicroSeconds;
  double angleLimitCW;
  double angleLimitCCW;
  unsigned char tempLimitHighCelcius;
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
  unsigned char statusRetLevel;

  unsigned char alarmLed;
  unsigned char alarmShutdown;

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
  unsigned char presentTemp;
  bool isInstructionRegistered;
  bool isMoving;
  bool isEepromLocked;

  // apparently this value is unused
//  unsigned char punch;

  MX28Snapshot() {}

  bool init(Robot::CM730& cm730, int const mx28ID);
};

#endif
