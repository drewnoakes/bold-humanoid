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
  auto modelNumber      = readTableWord(table, MX28::P_MODEL_NUMBER_L);
  auto version          = table[MX28::P_VERSION];
  auto id               = table[MX28::P_ID];
  auto baud             = table[MX28::P_BAUD_RATE];
  auto retDelayTime     = table[MX28::P_RETURN_DELAY_TIME];
  auto angleLimitCW     = readTableWord(table, MX28::P_CW_ANGLE_LIMIT_L);
  auto angleLimitCCW    = readTableWord(table, MX28::P_CCW_ANGLE_LIMIT_L);
  auto tempLimitHigh    = table[MX28::P_HIGH_LIMIT_TEMPERATURE];
  auto voltageLimitLow  = table[MX28::P_LOW_LIMIT_VOLTAGE];
  auto voltageLimitHigh = table[MX28::P_HIGH_LIMIT_VOLTAGE];
  auto maxTorque        = readTableWord(table, MX28::P_MAX_TORQUE_L);
  auto retLevel         = table[MX28::P_RETURN_LEVEL];
  auto alarmLed         = table[MX28::P_RETURN_LEVEL];
  auto alarmShutdown    = table[MX28::P_RETURN_LEVEL];
  //
  // RAM AREA
  //
  auto torqueEnable     = table[MX28::P_TORQUE_ENABLE];
  auto led              = table[MX28::P_LED];
  auto gainD            = table[MX28::P_D_GAIN];
  auto gainI            = table[MX28::P_I_GAIN];
  auto gainP            = table[MX28::P_P_GAIN];
  auto goalPosition     = readTableWord(table, MX28::P_GOAL_POSITION_L);
  auto movingSpeed      = readTableWord(table, MX28::P_MOVING_SPEED_L);
  auto torqueLimit      = readTableWord(table, MX28::P_TORQUE_LIMIT_L);
  auto presentPosition  = readTableWord(table, MX28::P_PRESENT_POSITION_L);
  auto presentSpeed     = readTableWord(table, MX28::P_PRESENT_SPEED_L);
  auto presentLoad      = readTableWord(table, MX28::P_PRESENT_LOAD_L);
  auto presentVoltage   = table[MX28::P_PRESENT_VOLTAGE];
  auto presentTemp      = table[MX28::P_PRESENT_TEMPERATURE];
  auto registeredInstr  = table[MX28::P_REGISTERED_INSTRUCTION];
  auto moving           = table[MX28::P_MOVING];
  auto lock             = table[MX28::P_LOCK];
  auto punch            = readTableWord(table, MX28::P_PUNCH_L);

  return true;
}