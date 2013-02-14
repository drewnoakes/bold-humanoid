#include <cstdio>

#include "CM730Snapshot.hh"

using namespace Robot;
using namespace Eigen;

unsigned short CM730Snapshot::readTableWord(unsigned char* table, int addr)
{
  return CM730::MakeWord(table[addr], table[addr+1]);
}

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

bool CM730Snapshot::init(Robot::CM730& cm730)
{
  unsigned char table[128];

  // Read the whole table in a single go
  if(cm730.ReadTable(CM730::ID_CM, CM730::P_MODEL_NUMBER_L, CM730::P_RIGHT_MIC_H, &table[CM730::P_MODEL_NUMBER_L], 0) != CM730::SUCCESS)
  {
    printf("Cannot read CM730 table.\n");
    return false;
  }

  // documentation: http://support.robotis.com/en/product/darwin-op/references/reference/hardware_specifications/electronics/sub_controller_(cm-730).htm

  //
  // EEPROM AREA
  //

  modelNumber     = readTableWord(table, CM730::P_MODEL_NUMBER_L); // 0x7300

  firmwareVersion = table[CM730::P_VERSION];

  dynamixelId     = table[CM730::P_ID];                            // 0xC8

  auto baudByte   = table[CM730::P_BAUD_RATE];                     // 0x01
  baudBPS = 2000000/(baudByte+1);

  auto retDelayTime = table[CM730::P_RETURN_DELAY_TIME];           // 0x00
  returnDelayTimeMicroSeconds = (unsigned int)retDelayTime * 2;

  statusRetLevel = table[CM730::P_RETURN_LEVEL];                         // 0x02

  //
  // RAM AREA
  //

  isPowered = table[CM730::P_DXL_POWER] == 1;

  auto ledPanel = table[CM730::P_LED_PANNEL];
  isLed2On = (ledPanel & 0x1) != 0;
  isLed3On = (ledPanel & 0x2) != 0;
  isLed4On = (ledPanel & 0x4) != 0;

  auto ledForehead = readTableWord(table, CM730::P_LED_HEAD_L);
  foreheadColor = shortToColour(ledForehead);

  auto ledEye = readTableWord(table, CM730::P_LED_EYE_L);
  eyeColor = shortToColour(ledEye);

  auto buttons      = table[CM730::P_BUTTON];
  isModeButtonPressed = (buttons & 0x1) != 0;
  isStartButtonPressed = (buttons & 0x2) != 0;

  auto gyroZ = readTableWord(table, CM730::P_GYRO_Z_L);
  auto gyroY = readTableWord(table, CM730::P_GYRO_Y_L);
  auto gyroX = readTableWord(table, CM730::P_GYRO_X_L);
  gyro = Vector3d(
    gyroValueToRps(gyroX),
    gyroValueToRps(gyroY),
    gyroValueToRps(gyroZ)
  );

  auto accX = readTableWord(table, CM730::P_ACCEL_X_L);
  auto accY = readTableWord(table, CM730::P_ACCEL_Y_L);
  auto accZ = readTableWord(table, CM730::P_ACCEL_Z_L);
  acc = Vector3d(
    accValueToGs(accX),
    accValueToGs(accY),
    accValueToGs(accZ)
  );

  voltage = table[CM730::P_VOLTAGE] / 10.0f;

  micLevelLeft = readTableWord(table, CM730::P_LEFT_MIC_L);
  micLevelRight = readTableWord(table, CM730::P_RIGHT_MIC_L);

  return true;
}
