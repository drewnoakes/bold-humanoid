#include "mx28.hh"

#include "../Math/math.hh"
#include "../util/assert.hh"

using namespace bold;
using namespace std;

ushort MX28::getMirrorValue(ushort value) { return (MAX_VALUE + 1 - value) & MAX_VALUE; }
double MX28::getMirrorAngle(double angle) { return -angle; }

ushort MX28::degs2Value(double angle) { return static_cast<ushort>(Math::clamp(static_cast<int>(round(angle*RATIO_DEGS2VALUE)), -CENTER_VALUE, MAX_VALUE-CENTER_VALUE) + CENTER_VALUE); }
double MX28::value2Degs(ushort value) { return (double)((int)value-(int)CENTER_VALUE)*RATIO_VALUE2DEGS; }

ushort MX28::rads2Value(double angle) { return static_cast<ushort>(Math::clamp(static_cast<int>(round(angle*RATIO_RADS2VALUE)), -CENTER_VALUE, MAX_VALUE-CENTER_VALUE) + CENTER_VALUE); }
double MX28::value2Rads(ushort value) { return (double)((int)value-(int)CENTER_VALUE)*RATIO_VALUE2RADS; }

ushort MX28::clampValue(int value) { return (ushort)Math::clamp(value, 0, (int)MAX_VALUE); }

/*
// TODO compare these old conversions with those being used for accuracy before deleting this commented code

double MX28::angleValueToRads(ushort value)
{
  return ((int)value - 0x800) * (M_PI / 0x800);
}

double MX28::valueToRPM(ushort value)
{
  // Value is between 0 & 1023 (0x3FF). Unit of 0.053 rpm.
  // If zero, maximum RPM of motor is used without speed control.
  // If 1023, about 54RPM (CW)
  // Between 1024 * 2047, values are CCW.
  // That is, bit 10 indicates the direction. Therefore both 0 and 1024 equal zero RPM.
  if (value < 1024)
    return value * 0.052733333;
  else
    return ((int)value - 1024) * 0.052733333;
}
*/

ushort MX28::rpm2Value(double speed) { int temp = ((int)(fabs(speed)*RATIO_RPM2VALUE)) & 0x3FF; if (speed < 0) temp |= 0x400; return temp; }
double MX28::value2Rpm(ushort value) { double temp = (value & 0x3FF)*RATIO_VALUE2RPM; if (value & 0x400) temp = -temp; return temp; }

ushort MX28::torque2Value(double speed) { int temp = ((int)(fabs(speed)*RATIO_TORQUE2VALUE)) & 0x3FF; if (speed < 0) temp |= 0x400; return temp; }
double MX28::value2Torque(ushort value) { double temp = (value & 0x3FF)*RATIO_VALUE2TORQUE; if (value & 0x400) temp = -temp; return temp; }

uchar MX28::centigrade2Value(int degreesCentigrade) { ASSERT(degreesCentigrade > 0); ASSERT(degreesCentigrade < 256); return degreesCentigrade; }
int MX28::value2Centigrade(uchar value) { return value; }

uchar MX28::voltage2Value(double volts) { ASSERT(volts > 0); ASSERT(volts < 25); return static_cast<uchar>(volts*10); }
double MX28::value2Voltage(uchar value) { return value / 10.0; }

double MX28::value2Load(ushort value) { return (value < 1024 ? value : value - 1024) / 1023.0; }

std::string MX28::getAddressName(MX28Table address)
{
  switch (address)
  {
    case MX28Table::MODEL_NUMBER_L:          return "MODEL_NUMBER";
    case MX28Table::VERSION:                 return "VERSION";
    case MX28Table::ID:                      return "ID";
    case MX28Table::BAUD_RATE:               return "BAUD_RATE";
    case MX28Table::RETURN_DELAY_TIME:       return "RETURN_DELAY_TIME";
    case MX28Table::CW_ANGLE_LIMIT_L:        return "CW_ANGLE_LIMIT";
    case MX28Table::CCW_ANGLE_LIMIT_L:       return "CCW_ANGLE_LIMIT";
    case MX28Table::SYSTEM_DATA2:            return "SYSTEM_DATA2";
    case MX28Table::LOW_LIMIT_VOLTAGE:       return "LOW_LIMIT_VOLTAGE";
    case MX28Table::MAX_TORQUE_L:            return "MAX_TORQUE";
    case MX28Table::RETURN_LEVEL:            return "RETURN_LEVEL";
    case MX28Table::ALARM_LED:               return "ALARM_LED";
    case MX28Table::ALARM_SHUTDOWN:          return "ALARM_SHUTDOWN";
    case MX28Table::OPERATING_MODE:          return "OPERATING_MODE";
    case MX28Table::LOW_CALIBRATION_L:       return "LOW_CALIBRATION";
    case MX28Table::HIGH_CALIBRATION_L:      return "HIGH_CALIBRATION";
    case MX28Table::TORQUE_ENABLE:           return "TORQUE_ENABLE";
    case MX28Table::LED:                     return "LED";
    case MX28Table::D_GAIN:                  return "D_GAIN";
    case MX28Table::I_GAIN:                  return "I_GAIN";
    case MX28Table::P_GAIN:                  return "P_GAIN";
    case MX28Table::RESERVED:                return "RESERVED";
    case MX28Table::GOAL_POSITION_L:         return "GOAL_POSITION";
    case MX28Table::MOVING_SPEED_L:          return "MOVING_SPEED";
    case MX28Table::TORQUE_LIMIT_L:          return "TORQUE_LIMIT";
    case MX28Table::PRESENT_POSITION_L:      return "PRESENT_POSITION";
    case MX28Table::PRESENT_SPEED_L:         return "PRESENT_SPEED";
    case MX28Table::PRESENT_LOAD_L:          return "PRESENT_LOAD";
    case MX28Table::PRESENT_VOLTAGE:         return "PRESENT_VOLTAGE";
    case MX28Table::PRESENT_TEMPERATURE:     return "PRESENT_TEMPERATURE";
    case MX28Table::REGISTERED_INSTRUCTION:  return "REGISTERED_INSTRUCTION";
    case MX28Table::PAUSE_TIME:              return "PAUSE_TIME";
    case MX28Table::MOVING:                  return "MOVING";
    case MX28Table::LOCK:                    return "LOCK";
    case MX28Table::PUNCH_L:                 return "PUNCH";
    case MX28Table::RESERVED4:               return "RESERVED4";
    case MX28Table::RESERVED5:               return "RESERVED5";
    case MX28Table::POT_L:                   return "POT";
    case MX28Table::PWM_OUT_L:               return "PWM_OUT";
    case MX28Table::P_ERROR_L:               return "P_ERROR";
    case MX28Table::I_ERROR_L:               return "I_ERROR";
    case MX28Table::D_ERROR_L:               return "D_ERROR";
    case MX28Table::P_ERROR_OUT_L:           return "P_ERROR_OUT";
    case MX28Table::I_ERROR_OUT_L:           return "I_ERROR_OUT";
    case MX28Table::D_ERROR_OUT_H:           return "D_ERROR_OUT";

    default: return "(UNKNOWN)";
  }
}
