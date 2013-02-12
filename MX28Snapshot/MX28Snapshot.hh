#ifndef BOLD_MX28_SNAPSHOT_HH
#define BOLD_MX28_SNAPSHOT_HH

#include "../robotis/Framework/include/CM730.h"

using namespace Robot;

class MX28Snapshot
{
private:
  unsigned short readTableWord(unsigned char* table, int addr);

public:

  // TODO convert these values to other types where it makes sense (eg temp -> degrees) as in CM730 snapshot

  // EEPROM AREA

  unsigned short modelNumber;
  unsigned char version;
  unsigned char id;
  unsigned char baud;
  unsigned char retDelayTime;
  unsigned short angleLimitCW;
  unsigned short angleLimitCCW;
  unsigned char tempLimitHigh;
  unsigned char voltageLimitLow;
  unsigned char voltageLimitHigh;
  unsigned short maxTorque;
  unsigned char retLevel;
  unsigned char alarmLed;
  unsigned char alarmShutdown;

  // RAM AREA

  unsigned char torqueEnable;
  unsigned char led;
  unsigned char gainD;
  unsigned char gainI;
  unsigned char gainP;
  unsigned short goalPosition;
  unsigned short movingSpeed;
  unsigned short torqueLimit;
  unsigned short presentPosition;
  unsigned short presentSpeed;
  unsigned short presentLoad;
  unsigned char presentVoltage;
  unsigned char presentTemp;
  unsigned char registeredInstr;
  unsigned char moving;
  unsigned char lock;
  unsigned char punch;

  MX28Snapshot() {}

  bool init(Robot::CM730& cm730, int const mx82ID);
};

#endif
