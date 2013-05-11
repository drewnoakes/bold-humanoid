#include "cm730snapshot.hh"

#include <cstdio>

#include "../CM730/cm730.hh"

using namespace Eigen;
using namespace bold;

// see http://support.robotis.com/en/product/darwin-op/references/reference/hardware_specifications/electronics/sub_controller_(cm-730).htm

// TODO have constructors just store const byte[] and convert fields to properties that make conversions

CM730Snapshot::CM730Snapshot(BulkReadTable const& data)
{
  // we don't read anything from the EEPROM area

  //
  // RAM AREA
  //

  isPowered = data.readByte(CM730::P_DXL_POWER) == 1;

  auto ledPanel = data.readByte(CM730::P_LED_PANNEL);
  isLed2On = (ledPanel & 0x1) != 0;
  isLed3On = (ledPanel & 0x2) != 0;
  isLed4On = (ledPanel & 0x4) != 0;

  auto ledForehead = data.readWord(CM730::P_LED_HEAD_L);
  foreheadColor = CM730::shortToColour(ledForehead);

  auto ledEye = data.readWord(CM730::P_LED_EYE_L);
  eyeColor = CM730::shortToColour(ledEye);

  auto buttons = data.readByte(CM730::P_BUTTON);
  isModeButtonPressed = (buttons & 0x1) != 0;
  isStartButtonPressed = (buttons & 0x2) != 0;

  auto gyroZ = data.readWord(CM730::P_GYRO_Z_L);
  auto gyroY = data.readWord(CM730::P_GYRO_Y_L);
  auto gyroX = data.readWord(CM730::P_GYRO_X_L);
  gyroRaw = Vector3i(gyroX, gyroY, gyroZ);
  gyro = Vector3d(
    CM730::gyroValueToRps(gyroX),
    CM730::gyroValueToRps(gyroY),
    CM730::gyroValueToRps(gyroZ)
  );

  auto accX = data.readWord(CM730::P_ACCEL_X_L);
  auto accY = data.readWord(CM730::P_ACCEL_Y_L);
  auto accZ = data.readWord(CM730::P_ACCEL_Z_L);
  accRaw = Vector3i(accX, accY, accZ);
  acc = Vector3d(
    CM730::accValueToGs(accX),
    CM730::accValueToGs(accY),
    CM730::accValueToGs(accZ)
  );

  voltage = data.readByte(CM730::P_VOLTAGE) / 10.0f;
}

StaticCM730State::StaticCM730State(BulkReadTable const& data)
{
  //
  // EEPROM AREA
  //

  modelNumber     = data.readWord(CM730::P_MODEL_NUMBER_L);       // 0x7300

  firmwareVersion = data.readByte(CM730::P_VERSION);

  dynamixelId     = data.readByte(CM730::P_ID);                   // 0xC8

  auto baudByte   = data.readByte(CM730::P_BAUD_RATE);            // 0x01
  baudBPS = 2000000/(baudByte+1);

  auto retDelayTime = data.readByte(CM730::P_RETURN_DELAY_TIME);  // 0x00
  returnDelayTimeMicroSeconds = (unsigned int)retDelayTime * 2;

  statusRetLevel = data.readByte(CM730::P_RETURN_LEVEL);          // 0x02

  // we don't read anything from the RAM area
}
