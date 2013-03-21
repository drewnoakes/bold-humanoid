#include <cstdio>

#include "CM730Snapshot.hh"

using namespace Robot;
using namespace Eigen;
using namespace bold;

double CM730Snapshot::gyroValueToDps(int value)
{
  // TODO the range in the positive and negative should be slightly different
  // see http://support.robotis.com/en/product/darwin-op/references/reference/hardware_specifications/electronics/sub_controller_(cm-730).htm
  return ((value-512)/512.0)*1600.0;
}

double CM730Snapshot::gyroValueToRps(int value)
{
  // 0 -> -1600 dps
  // 512 -> 0 dps
  // 1023 -> +1600 dps
  double dps = gyroValueToDps(value);
  return dps*M_PI/180.0;
}

double CM730Snapshot::accValueToGs(int value)
{
  // TODO the range in the positive and negative should be slightly different
  // see http://support.robotis.com/en/product/darwin-op/references/reference/hardware_specifications/electronics/sub_controller_(cm-730).htm
  return ((value-512)/512.0)*4.0;
}

Vector3d CM730Snapshot::shortToColour(unsigned short s)
{
  auto r =  s        & 0x1F; // first five bits
  auto g = (s >> 5)  & 0x1F; // next five bits
  auto b = (s >> 10) & 0x1F; // next five bits

  return Vector3d(
    r / 31.0,
    g / 31.0,
    b / 31.0);
}

bool CM730Snapshot::init(Robot::BulkReadData& data)
{
  // documentation: http://support.robotis.com/en/product/darwin-op/references/reference/hardware_specifications/electronics/sub_controller_(cm-730).htm

  //
  // EEPROM AREA
  //

  modelNumber     = data.ReadWord(CM730::P_MODEL_NUMBER_L); // 0x7300

  firmwareVersion = data.ReadByte(CM730::P_VERSION);

  dynamixelId     = data.ReadByte(CM730::P_ID);                            // 0xC8

  auto baudByte   = data.ReadByte(CM730::P_BAUD_RATE);                     // 0x01
  baudBPS = 2000000/(baudByte+1);

  auto retDelayTime = data.ReadByte(CM730::P_RETURN_DELAY_TIME);           // 0x00
  returnDelayTimeMicroSeconds = (unsigned int)retDelayTime * 2;

  statusRetLevel = data.ReadByte(CM730::P_RETURN_LEVEL);                         // 0x02

  //
  // RAM AREA
  //

  isPowered = data.ReadByte(CM730::P_DXL_POWER) == 1;

  auto ledPanel = data.ReadByte(CM730::P_LED_PANNEL);
  isLed2On = (ledPanel & 0x1) != 0;
  isLed3On = (ledPanel & 0x2) != 0;
  isLed4On = (ledPanel & 0x4) != 0;

  auto ledForehead = data.ReadWord(CM730::P_LED_HEAD_L);
  foreheadColor = shortToColour(ledForehead);

  auto ledEye = data.ReadWord(CM730::P_LED_EYE_L);
  eyeColor = shortToColour(ledEye);

  auto buttons      = data.ReadByte(CM730::P_BUTTON);
  isModeButtonPressed = (buttons & 0x1) != 0;
  isStartButtonPressed = (buttons & 0x2) != 0;

  auto gyroZ = data.ReadWord(CM730::P_GYRO_Z_L);
  auto gyroY = data.ReadWord(CM730::P_GYRO_Y_L);
  auto gyroX = data.ReadWord(CM730::P_GYRO_X_L);
  gyro = Vector3d(
    gyroValueToRps(gyroX),
    gyroValueToRps(gyroY),
    gyroValueToRps(gyroZ)
  );

  auto accX = data.ReadWord(CM730::P_ACCEL_X_L);
  auto accY = data.ReadWord(CM730::P_ACCEL_Y_L);
  auto accZ = data.ReadWord(CM730::P_ACCEL_Z_L);
  acc = Vector3d(
    accValueToGs(accX),
    accValueToGs(accY),
    accValueToGs(accZ)
  );

  voltage = data.ReadByte(CM730::P_VOLTAGE) / 10.0f;

  micLevelLeft = data.ReadWord(CM730::P_LEFT_MIC_L);
  micLevelRight = data.ReadWord(CM730::P_RIGHT_MIC_L);

  return true;
}
