#include <cstdio>

#include "MX28Snapshot.hh"

unsigned short MX28Snapshot::readTableWord(unsigned char* table, int addr)
{
  return CM730::MakeWord(table[addr], table[addr+1]);
}

bool MX28Snapshot::init(Robot::CM730 cm730, int mx82ID)
{
  unsigned char table[128];

  // NOTE there are values above PUNCH_H for input/output of PID error components, but we ignore them here

  if(cm730.ReadTable(mx82ID, MX28::P_MODEL_NUMBER_L, MX28::P_PUNCH_H, &table[MX28::P_MODEL_NUMBER_L], 0) != CM730::SUCCESS)
  {
    printf("Cannot read MX28 table: %d\n", mx82ID);
    return false;
  }

  //
  // EEPROM AREA
  //
  modelNumber      = readTableWord(table, MX28::P_MODEL_NUMBER_L);
  version          = table[MX28::P_VERSION];
  id               = table[MX28::P_ID];
  baud             = table[MX28::P_BAUD_RATE];
  retDelayTime     = table[MX28::P_RETURN_DELAY_TIME];
  angleLimitCW     = readTableWord(table, MX28::P_CW_ANGLE_LIMIT_L);
  angleLimitCCW    = readTableWord(table, MX28::P_CCW_ANGLE_LIMIT_L);
  tempLimitHigh    = table[MX28::P_HIGH_LIMIT_TEMPERATURE];
  voltageLimitLow  = table[MX28::P_LOW_LIMIT_VOLTAGE];
  voltageLimitHigh = table[MX28::P_HIGH_LIMIT_VOLTAGE];
  maxTorque        = readTableWord(table, MX28::P_MAX_TORQUE_L);
  retLevel         = table[MX28::P_RETURN_LEVEL];
  alarmLed         = table[MX28::P_RETURN_LEVEL];
  alarmShutdown    = table[MX28::P_RETURN_LEVEL];
  //
  // RAM AREA
  //
  torqueEnable     = table[MX28::P_TORQUE_ENABLE];
  led              = table[MX28::P_LED];
  gainD            = table[MX28::P_D_GAIN];
  gainI            = table[MX28::P_I_GAIN];
  gainP            = table[MX28::P_P_GAIN];
  goalPosition     = readTableWord(table, MX28::P_GOAL_POSITION_L);
  movingSpeed      = readTableWord(table, MX28::P_MOVING_SPEED_L);
  torqueLimit      = readTableWord(table, MX28::P_TORQUE_LIMIT_L);
  presentPosition  = readTableWord(table, MX28::P_PRESENT_POSITION_L);
  presentSpeed     = readTableWord(table, MX28::P_PRESENT_SPEED_L);
  presentLoad      = readTableWord(table, MX28::P_PRESENT_LOAD_L);
  presentVoltage   = table[MX28::P_PRESENT_VOLTAGE];
  presentTemp      = table[MX28::P_PRESENT_TEMPERATURE];
  registeredInstr  = table[MX28::P_REGISTERED_INSTRUCTION];
  moving           = table[MX28::P_MOVING];
  lock             = table[MX28::P_LOCK];
  punch            = readTableWord(table, MX28::P_PUNCH_L);

  return true;
}