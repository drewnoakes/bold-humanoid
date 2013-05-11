#include "mx28snapshot.hh"

#include <cassert>
#include <cstdio>
#include <cmath>

#include "../MX28/mx28.hh"
#include "../CM730/cm730.hh"

using namespace bold;

// see http://support.robotis.com/en/product/dynamixel/rx_series/mx-28.htm

// TODO have constructors just store const byte[] and convert fields to properties that make conversions

MX28Snapshot::MX28Snapshot(BulkReadTable const& data, int const mx28ID)
{
  id = data.readByte(MX28::P_ID);

  assert(id == mx28ID);

  presentPositionValue = data.readWord(MX28::P_PRESENT_POSITION_L);
  presentPosition = MX28::value2Rads(presentPositionValue);

  presentSpeedRPM = MX28::value2Rpm(data.readWord(MX28::P_PRESENT_SPEED_L));

  // TODO migrate to MX28::value2Load(int)
  auto presentLoadInt = data.readWord(MX28::P_PRESENT_LOAD_L);
  if (presentLoadInt < 1024)
    presentLoad = presentLoadInt / 1023.0;
  else
    presentLoad = (presentLoadInt - 1024) / 1023.0;

  // TODO migrate to MX28::value2Voltage(uchar)
  presentVoltage = data.readByte(MX28::P_PRESENT_VOLTAGE) * 0.1;

  presentTemp = data.readByte(MX28::P_PRESENT_TEMPERATURE);
}

StaticMX28State::StaticMX28State(BulkReadTable const& data, int const mx28ID)
{
  //
  // EEPROM AREA
  //
  modelNumber      = data.readWord(MX28::P_MODEL_NUMBER_L);    // 0x001D
  firmwareVersion  = data.readByte(MX28::P_VERSION);
  id               = data.readByte(MX28::P_ID);

  assert(id == mx28ID);

  // TODO migrate to MX28::value2Baud(uchar)
  auto baudByte    = data.readByte(MX28::P_BAUD_RATE);                        // 0x22
  baudBPS = 2000000/(baudByte+1);

  // TODO migrate to MX28::value2ReturnDelayTime(uchar)
  auto retDelayTime = data.readByte(MX28::P_RETURN_DELAY_TIME);               // 0x00
  returnDelayTimeMicroSeconds = (unsigned int)retDelayTime * 2;

  // If both are set to zero, the servo is in wheel mode
  angleLimitCW     = MX28::value2Rads(data.readWord(MX28::P_CW_ANGLE_LIMIT_L));  // 0x0000
  angleLimitCCW    = MX28::value2Rads(data.readWord(MX28::P_CCW_ANGLE_LIMIT_L)); // 0x0FFF

  // If temp passes this limit, Heating Error Bit (Bit2) of status packet is set, and alarm is triggered
  tempLimitHighCelcius = data.readByte(MX28::P_HIGH_LIMIT_TEMPERATURE);       // 0x50 (80 degrees)

  // If voltage passes these limits, Voltage Range Error Bit (Bit0) of status packet is set, and alarm is triggered
  voltageLimitLow  = data.readByte(MX28::P_LOW_LIMIT_VOLTAGE) * 0.1;          // 0x3C (6.0 V)
  voltageLimitHigh = data.readByte(MX28::P_HIGH_LIMIT_VOLTAGE) * 0.1;         // 0xA0 (16.0 V)

  // As a ratio of the max torque output, between 0 and 1
  maxTorque        = data.readWord(MX28::P_MAX_TORQUE_L) / (double)0x03FF; // 0x03FF

  statusRetLevel   = data.readByte(MX28::P_RETURN_LEVEL);                     // 0x02

  // Alarms use a bitmask
  //
  // bit 0 - input voltage error - applied voltage is outside MX28::P_LOW_LIMIT_VOLTAGE and MX28::P_HIGH_LIMIT_VOLTAGE
  // bit 1 - angle limit error - goal position is outside MX28::P_CW_ANGLE_LIMIT_L and MX28::P_CCW_ANGLE_LIMIT_L
  // bit 2 - overheating error - internal temperature exceeds the limit set via MX28::P_HIGH_LIMIT_TEMPERATURE
  // bit 3 - range error - command is given beyond range of usage
  // bit 4 - checksum error - the checksum of the received instruction packet is incorrect
  // bit 5 - overoad error - the current load cannot be controlled with the set maximum torque
  // bit 6 - instruction error - undefined instruction, or action command delivered without reg_write
  // bit 7 - unused?

  alarmLed         = MX28Alarm(data.readByte(MX28::P_ALARM_LED));      // 0x24
  alarmShutdown    = MX28Alarm(data.readByte(MX28::P_ALARM_SHUTDOWN)); // 0x24

  //
  // RAM AREA
  //
  torqueEnable     = data.readByte(MX28::P_TORQUE_ENABLE) != 0;
  led              = data.readByte(MX28::P_LED) != 0;

  gainP            = data.readByte(MX28::P_P_GAIN) / 8.0;
  gainI            = data.readByte(MX28::P_I_GAIN) * 1000 / 2048.0;
  gainD            = data.readByte(MX28::P_D_GAIN) * 4 / 1000.0;

  // Value is between 0 & 7095 (0xFFF). Unit of 0.088 degrees.
  goalPositionRads = MX28::value2Rads(data.readWord(MX28::P_GOAL_POSITION_L));

  movingSpeedRPM   = MX28::value2Rpm(data.readWord(MX28::P_MOVING_SPEED_L));

  // Percentage of max torque to use as a limit. 0 to 1023 (0x3FF). Unit about 0.1%.
  torqueLimit      = data.readWord(MX28::P_TORQUE_LIMIT_L) / 1023.0;

  isInstructionRegistered = data.readByte(MX28::P_REGISTERED_INSTRUCTION) != 0; // 0x00
  isMoving                = data.readByte(MX28::P_MOVING) != 0;                 // 0x00
  isEepromLocked          = data.readByte(MX28::P_LOCK) != 0;                   // 0x00

  // apparently this value is unused
//  punch            = data.readWord(MX28::P_PUNCH_L);             // 0x0020
}
