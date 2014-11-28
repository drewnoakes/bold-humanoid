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

  isPowered = data.readByte(CM730Table::DXL_POWER) == 1;

  auto ledPanel = data.readByte(CM730Table::LED_PANEL);
  isLed2On = (ledPanel & 0x1) != 0; // red
  isLed3On = (ledPanel & 0x2) != 0; // blue
  isLed4On = (ledPanel & 0x4) != 0; // green

  auto ledForehead = data.readWord(CM730Table::LED_HEAD_L);
  foreheadColor = CM730::shortToColour(ledForehead);

  auto ledEye = data.readWord(CM730Table::LED_EYE_L);
  eyeColor = CM730::shortToColour(ledEye);

  auto buttons = data.readByte(CM730Table::BUTTON);
  isModeButtonPressed = (buttons & 0x1) != 0;
  isStartButtonPressed = (buttons & 0x2) != 0;

  // TODO when IMU calibration data available, use it here
  // NOTE the gyro uses a different set of axes than the agent frame,
  //      so we swap and flip the X & Y axes here to make them identical.
  auto gyroX = data.readWord(CM730Table::GYRO_Y_L);                      // X =  Y
  auto gyroY = CM730::flipImuValue(data.readWord(CM730Table::GYRO_X_L)); // Y = -X
  auto gyroZ = data.readWord(CM730Table::GYRO_Z_L);                      // Z =  Z
  gyroRaw = Vector3i(gyroX, gyroY, gyroZ);
  gyro = Vector3d(
    CM730::gyroValueToRps(gyroX),
    CM730::gyroValueToRps(gyroY),
    CM730::gyroValueToRps(gyroZ)
  );

  // TODO when IMU calibration data available, use it here
  auto accX = data.readWord(CM730Table::ACCEL_X_L);
  auto accY = data.readWord(CM730Table::ACCEL_Y_L);
  auto accZ = data.readWord(CM730Table::ACCEL_Z_L);
  accRaw = Vector3i(accX, accY, accZ);
  acc = Vector3d(
    CM730::accValueToGs(accX),
    CM730::accValueToGs(accY),
    CM730::accValueToGs(accZ)
  );

  voltage = data.readByte(CM730Table::VOLTAGE) / 10.0f;
}

StaticCM730State::StaticCM730State(BulkReadTable const& data)
{
  //
  // EEPROM AREA
  //

  modelNumber     = data.readWord(CM730Table::MODEL_NUMBER_L);       // 0x7300

  firmwareVersion = data.readByte(CM730Table::VERSION);

  dynamixelId     = data.readByte(CM730Table::ID);                   // 0xC8

  ASSERT(dynamixelId == CM730::ID_CM);

  auto baudByte   = data.readByte(CM730Table::BAUD_RATE);            // 0x01
  baudBPS = 2000000/(baudByte+1);

  auto retDelayTime = data.readByte(CM730Table::RETURN_DELAY_TIME);  // 0x00
  returnDelayTimeMicroSeconds = (unsigned int)retDelayTime * 2;

  statusRetLevel = data.readByte(CM730Table::RETURN_LEVEL);          // 0x02

  // we don't read anything from the RAM area
}
