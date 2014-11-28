#include "mx28snapshot.hh"

#include <cstdio>
#include <cmath>

#include "../MX28/mx28.hh"
#include "../CM730/cm730.hh"
#include "../util/assert.hh"

using namespace bold;

// see http://support.robotis.com/en/product/dynamixel/rx_series/mx-28.htm

// TODO have constructors just store const byte[] and convert fields to properties that make conversions

MX28Snapshot::MX28Snapshot(uchar mx28ID, BulkReadTable const& data, int jointOffset)
{
  id = mx28ID;
  presentPositionValue = static_cast<ushort>(Math::clamp(data.readWord(MX28Table::PRESENT_POSITION_L) - jointOffset, 0, (int)MX28::MAX_VALUE));
  presentPosition = MX28::value2Rads(presentPositionValue);
  presentSpeedRPM = MX28::value2Rpm(data.readWord(MX28Table::PRESENT_SPEED_L));
  presentLoad = MX28::value2Load(data.readWord(MX28Table::PRESENT_LOAD_L));
  presentVoltage = MX28::value2Voltage(data.readByte(MX28Table::PRESENT_VOLTAGE));
  presentTemp = MX28::value2Centigrade(data.readByte(MX28Table::PRESENT_TEMPERATURE));
}

StaticMX28State::StaticMX28State(uchar mx28ID, BulkReadTable const& data)
{
  //
  // EEPROM AREA
  //
  modelNumber      = data.readWord(MX28Table::MODEL_NUMBER_L);    // 0x001D
  firmwareVersion  = data.readByte(MX28Table::VERSION);
  id               = data.readByte(MX28Table::ID);

  ASSERT(id == mx28ID);

  // TODO migrate to MX28::value2Baud(uchar)
  auto baudByte    = data.readByte(MX28Table::BAUD_RATE);                        // 0x22
  baudBPS = 2000000u/(baudByte+1);

  // TODO migrate to MX28::value2ReturnDelayTime(uchar)
  auto retDelayTime = data.readByte(MX28Table::RETURN_DELAY_TIME);               // 0x00
  returnDelayTimeMicroSeconds = (unsigned int)retDelayTime * 2;

  // If both are set to zero, the servo is in wheel mode
  angleLimitCW     = MX28::value2Rads(data.readWord(MX28Table::CW_ANGLE_LIMIT_L));  // 0x0000
  angleLimitCCW    = MX28::value2Rads(data.readWord(MX28Table::CCW_ANGLE_LIMIT_L)); // 0x0FFF

  // If temp passes this limit, Heating Error Bit (Bit2) of status packet is set, and alarm is triggered
  tempLimitHighCelsius = data.readByte(MX28Table::HIGH_LIMIT_TEMPERATURE);       // 0x50 (80 degrees)

  // If voltage passes these limits, Voltage Range Error Bit (Bit0) of status packet is set, and alarm is triggered
  voltageLimitLow  = data.readByte(MX28Table::LOW_LIMIT_VOLTAGE) * 0.1;          // 0x3C (6.0 V)
  voltageLimitHigh = data.readByte(MX28Table::HIGH_LIMIT_VOLTAGE) * 0.1;         // 0xA0 (16.0 V)

  // As a ratio of the max torque output, between 0 and 1
  maxTorque        = data.readWord(MX28Table::MAX_TORQUE_L) / (double)0x03FF; // 0x03FF

  statusRetLevel   = data.readByte(MX28Table::RETURN_LEVEL);                     // 0x02

  // Alarms use a bitmask
  //
  // bit 0 - input voltage error - applied voltage is outside MX28Table::LOW_LIMIT_VOLTAGE and MX28Table::HIGH_LIMIT_VOLTAGE
  // bit 1 - angle limit error - goal position is outside MX28Table::CW_ANGLE_LIMIT_L and MX28Table::CCW_ANGLE_LIMIT_L
  // bit 2 - overheating error - internal temperature exceeds the limit set via MX28Table::HIGH_LIMIT_TEMPERATURE
  // bit 3 - range error - command is given beyond range of usage
  // bit 4 - checksum error - the checksum of the received instruction packet is incorrect
  // bit 5 - overload error - the current load cannot be controlled with the set maximum torque
  // bit 6 - instruction error - undefined instruction, or action command delivered without reg_write
  // bit 7 - unused?

  alarmLed         = MX28Alarm(data.readByte(MX28Table::ALARM_LED));      // 0x24
  alarmShutdown    = MX28Alarm(data.readByte(MX28Table::ALARM_SHUTDOWN)); // 0x24

  //
  // RAM AREA
  //
  torqueEnable     = data.readByte(MX28Table::TORQUE_ENABLE) != 0;
/*
  led              = data.readByte(MX28Table::LED) != 0;

  gainP            = data.readByte(MX28Table::P_GAIN) / 8.0;
  gainI            = data.readByte(MX28Table::I_GAIN) * 1000 / 2048.0;
  gainD            = data.readByte(MX28Table::D_GAIN) * 4 / 1000.0;

  // Value is between 0 & 7095 (0xFFF). Unit of 0.088 degrees.
  goalPositionRads = MX28::value2Rads(data.readWord(MX28Table::GOAL_POSITION_L));

  movingSpeedRPM   = MX28::value2Rpm(data.readWord(MX28Table::MOVING_SPEED_L));

  // Percentage of max torque to use as a limit. 0 to 1023 (0x3FF). Unit about 0.1%.
  torqueLimit      = data.readWord(MX28Table::TORQUE_LIMIT_L) / 1023.0;

  isInstructionRegistered = data.readByte(MX28Table::REGISTERED_INSTRUCTION) != 0; // 0x00
  isMoving                = data.readByte(MX28Table::MOVING) != 0;                 // 0x00
*/
  isEepromLocked          = data.readByte(MX28Table::LOCK) != 0;                   // 0x00

  // apparently this value is unused
//  punch            = data.readWord(MX28Table::PUNCH_L);             // 0x0020
}
