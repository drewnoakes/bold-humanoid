#include "mx28.hh"

#include "../Math/math.hh"

using namespace bold;
using namespace std;

//ushort MX28::getMirrorValue(ushort value) { return MAX_VALUE + 1 - value; }
//double MX28::getMirrorAngle(double angle) { return -angle; }

ushort MX28::degs2Value(double angle) { return (angle*RATIO_DEGS2VALUE)+CENTER_VALUE; }
double MX28::value2Degs(ushort value) { return (double)((int)value-(int)CENTER_VALUE)*RATIO_VALUE2DEGS; }

ushort MX28::rads2Value(double angle) { return (angle*RATIO_RADS2VALUE)+CENTER_VALUE; }
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
