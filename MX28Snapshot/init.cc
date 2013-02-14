#include <cstdio>
#include <cmath>

#include "MX28Snapshot.hh"

unsigned short MX28Snapshot::readTableWord(unsigned char* table, int addr)
{
  return CM730::MakeWord(table[addr], table[addr+1]);
}

double MX28Snapshot::angleValueToRads(unsigned int value)
{
  // see http://support.robotis.com/en/product/dynamixel/rx_series/mx-28.htm
  return (value - 0x800) / (M_PI * 2.0);
}

double MX28Snapshot::valueToRPM(unsigned int value)
{
  // see http://support.robotis.com/en/product/dynamixel/rx_series/mx-28.htm
  // Value is between 0 & 1023 (0x3FF). Unit of 0.053 rpm.
  // If zero, maximum RPM of motor is used without speed control.
  // If 1023, about 54RPM (CW)
  // Between 1024 * 2047, values are CCW.
  // That is, bit 10 indicates the direction. Therefore both 0 and 1024 equal zero RPM.
  if (value < 1024)
    return value * 0.052733333;
  else
    return (value - 1024) * 0.052733333;
}

bool MX28Snapshot::init(Robot::CM730& cm730, int const mx28ID)
{
  unsigned char table[128];

  // NOTE there are values above PUNCH_H for input/output of PID error components, but we ignore them here

  if(cm730.ReadTable(mx28ID, MX28::P_MODEL_NUMBER_L, MX28::P_PUNCH_H, &table[MX28::P_MODEL_NUMBER_L], 0) != CM730::SUCCESS)
  {
    printf("Cannot read MX28 table: %d\n", mx28ID);
    return false;
  }

  // documentation: http://support.robotis.com/en/product/dynamixel/rx_series/mx-28.htm

  //
  // EEPROM AREA
  //
  modelNumber      = readTableWord(table, MX28::P_MODEL_NUMBER_L);    // 0x001D
  firmwareVersion  = table[MX28::P_VERSION];
  id               = table[MX28::P_ID];

  auto baudByte    = table[MX28::P_BAUD_RATE];                        // 0x22
  baudBPS = 2000000/(baudByte+1);

  auto retDelayTime = table[MX28::P_RETURN_DELAY_TIME];               // 0x00
  returnDelayTimeMicroSeconds = (unsigned int)retDelayTime * 2;

  // If both are set to zero, the servo is in wheel mode
  angleLimitCW     = angleValueToRads(readTableWord(table, MX28::P_CW_ANGLE_LIMIT_L));  // 0x0000
  angleLimitCCW    = angleValueToRads(readTableWord(table, MX28::P_CCW_ANGLE_LIMIT_L)); // 0x0FFF

  // If temp passes this limit, Heating Error Bit (Bit2) of status packet is set, and alarm is triggered
  tempLimitHighCelcius = table[MX28::P_HIGH_LIMIT_TEMPERATURE];       // 0x50 (80 degrees)

  // If voltage passes these limits, Voltage Range Error Bit (Bit0) of status packet is set, and alarm is triggered
  voltageLimitLow  = table[MX28::P_LOW_LIMIT_VOLTAGE] * 0.1;          // 0x3C (6.0 V)
  voltageLimitHigh = table[MX28::P_HIGH_LIMIT_VOLTAGE] * 0.1;         // 0xA0 (16.0 V)

  // As a ratio of the max torque output, between 0 and 1
  maxTorque        = readTableWord(table, MX28::P_MAX_TORQUE_L) / (double)0x03FF; // 0x03FF

  statusRetLevel   = table[MX28::P_RETURN_LEVEL];                     // 0x02

  // Alarms use a bitmask, though I'm not sure on what ID it's present:
  //
  // bit 7 - unused?
  // bit 6 - instruction error - undefined instruction, or action command delivered without reg_write
  // bit 5 - overoad error - the current load cannot be controlled with the set maximum torque
  // bit 4 - checksum error - the checksum of the received instruction packet is incorrect
  // bit 3 - range error - command is given beyond range of usage
  // bit 2 - overheating error - internal temperature exceeds the limit set via MX28::P_HIGH_LIMIT_TEMPERATURE
  // bit 1 - angle limit error - goal position is outside MX28::P_CW_ANGLE_LIMIT_L and MX28::P_CCW_ANGLE_LIMIT_L
  // bit 0 - input voltage error - applied voltage is outside MX28::P_LOW_LIMIT_VOLTAGE and MX28::P_HIGH_LIMIT_VOLTAGE

  alarmLed         = table[MX28::P_ALARM_LED];                        // 0x24
  alarmShutdown    = table[MX28::P_ALARM_SHUTDOWN];                   // 0x24

  //
  // RAM AREA
  //
  torqueEnable     = table[MX28::P_TORQUE_ENABLE] != 0;
  led              = table[MX28::P_LED] != 0;

  gainP            = table[MX28::P_P_GAIN] / 8.0;
  gainI            = table[MX28::P_I_GAIN] * 1000 / 2048.0;
  gainD            = table[MX28::P_D_GAIN] * 4 / 1000.0;

  // Value is between 0 & 7095 (0xFFF). Unit of 0.088 degrees.
  goalPositionRads = angleValueToRads(readTableWord(table, MX28::P_GOAL_POSITION_L));

  movingSpeedRPM   = valueToRPM(readTableWord(table, MX28::P_MOVING_SPEED_L));

  // Percentage of max torque to use as a limit. 0 to 1023 (0x3FF). Unit about 0.1%.
  torqueLimit      = readTableWord(table, MX28::P_TORQUE_LIMIT_L) / 1023.0;

  presentPosition  = angleValueToRads(readTableWord(table, MX28::P_PRESENT_POSITION_L));

  presentSpeedRPM = valueToRPM(readTableWord(table, MX28::P_PRESENT_SPEED_L));

  auto presentLoadInt = readTableWord(table, MX28::P_PRESENT_LOAD_L);
  if (presentLoadInt < 1024)
    presentLoad = presentLoadInt / 1023.0;
  else
    presentLoad = (presentLoadInt - 1024) / 1023.0;

  presentVoltage   = table[MX28::P_PRESENT_VOLTAGE] * 0.1;

  presentTemp      = table[MX28::P_PRESENT_TEMPERATURE];

  isInstructionRegistered = table[MX28::P_REGISTERED_INSTRUCTION] != 0; // 0x00
  isMoving                = table[MX28::P_MOVING] != 0;                 // 0x00
  isEepromLocked          = table[MX28::P_LOCK] != 0;                   // 0x00

  // apparently this value is unused
//  punch            = readTableWord(table, MX28::P_PUNCH_L);             // 0x0020

  return true;
}
