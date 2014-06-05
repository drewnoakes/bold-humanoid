#include "mx28.hh"

#include "../Math/math.hh"
#include "../util/assert.hh"

using namespace bold;
using namespace std;

//ushort MX28::getMirrorValue(ushort value) { return MAX_VALUE + 1 - value; }
//double MX28::getMirrorAngle(double angle) { return -angle; }

ushort MX28::degs2Value(double angle) { return static_cast<ushort>(Math::clamp(static_cast<int>(round(angle*RATIO_DEGS2VALUE)), -CENTER_VALUE, MX28::MAX_VALUE-CENTER_VALUE) + CENTER_VALUE); }
double MX28::value2Degs(ushort value) { return (double)((int)value-(int)CENTER_VALUE)*RATIO_VALUE2DEGS; }

ushort MX28::rads2Value(double angle) { return static_cast<ushort>(Math::clamp(static_cast<int>(round(angle*RATIO_RADS2VALUE)), -CENTER_VALUE, MX28::MAX_VALUE-CENTER_VALUE) + CENTER_VALUE); }
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

std::string MX28::getAddressName(uchar address)
{
  switch (address)
  {
    case P_MODEL_NUMBER_L: return "MODEL_NUMBER";
    case P_VERSION: return "VERSION";
    case P_ID: return "ID";
    case P_BAUD_RATE: return "BAUD_RATE";
    case P_RETURN_DELAY_TIME: return "RETURN_DELAY_TIME";
    case P_CW_ANGLE_LIMIT_L: return "CW_ANGLE_LIMIT";
    case P_CCW_ANGLE_LIMIT_L: return "CCW_ANGLE_LIMIT";
    case P_SYSTEM_DATA2: return "SYSTEM_DATA2";
    case P_LOW_LIMIT_VOLTAGE: return "LOW_LIMIT_VOLTAGE";
    case P_MAX_TORQUE_L: return "MAX_TORQUE";
    case P_RETURN_LEVEL: return "RETURN_LEVEL";
    case P_ALARM_LED: return "ALARM_LED";
    case P_ALARM_SHUTDOWN: return "ALARM_SHUTDOWN";
    case P_OPERATING_MODE: return "OPERATING_MODE";
    case P_LOW_CALIBRATION_L: return "LOW_CALIBRATION";
    case P_HIGH_CALIBRATION_L: return "HIGH_CALIBRATION";
    case P_TORQUE_ENABLE: return "TORQUE_ENABLE";
    case P_LED: return "LED";
    case P_D_GAIN: return "D_GAIN";
    case P_I_GAIN: return "I_GAIN";
    case P_P_GAIN: return "P_GAIN";
    case P_RESERVED: return "RESERVED";
    case P_GOAL_POSITION_L: return "GOAL_POSITION";
    case P_MOVING_SPEED_L: return "MOVING_SPEED";
    case P_TORQUE_LIMIT_L: return "TORQUE_LIMIT";
    case P_PRESENT_POSITION_L: return "PRESENT_POSITION";
    case P_PRESENT_SPEED_L: return "PRESENT_SPEED";
    case P_PRESENT_LOAD_L: return "PRESENT_LOAD";
    case P_PRESENT_VOLTAGE: return "PRESENT_VOLTAGE";
    case P_PRESENT_TEMPERATURE: return "PRESENT_TEMPERATURE";
    case P_REGISTERED_INSTRUCTION: return "REGISTERED_INSTRUCTION";
    case P_PAUSE_TIME: return "PAUSE_TIME";
    case P_MOVING: return "MOVING";
    case P_LOCK: return "LOCK";
    case P_PUNCH_L: return "PUNCH";
    case P_RESERVED4: return "RESERVED4";
    case P_RESERVED5: return "RESERVED5";
    case P_POT_L: return "POT";
    case P_PWM_OUT_L: return "PWM_OUT";
    case P_P_ERROR_L: return "P_ERROR";
    case P_I_ERROR_L: return "I_ERROR";
    case P_D_ERROR_L: return "D_ERROR";
    case P_P_ERROR_OUT_L: return "P_ERROR_OUT";
    case P_I_ERROR_OUT_L: return "I_ERROR_OUT";
    case P_D_ERROR_OUT_H: return "D_ERROR_OUT";

    default: return "(UNKNOWN)";
  }
}
