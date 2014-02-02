#pragma once

#include <Eigen/Core>

namespace bold
{
  class BulkReadTable;

  // http://support.robotis.com/en/product/darwin-op/references/reference/hardware_specifications/electronics/sub_controller_(cm-730).htm

  // TODO rename as CM730State

  class CM730Snapshot
  {
  public:
    bool isPowered;
    /// Red LED
    bool isLed2On;
    /// Blue LED
    bool isLed3On;
    /// Green LED
    bool isLed4On;
    Eigen::Vector3d eyeColor;
    Eigen::Vector3d foreheadColor;
    bool isModeButtonPressed;
    bool isStartButtonPressed;
    /// The converted gyroscope output, in degrees per second.
    Eigen::Vector3d gyro;
    /// The converted accelerometer output, in gs.
    Eigen::Vector3d acc;
    float voltage;
    /// Raw raw value of the gyroscope, in range [0,1023] corresponding to [-1600,1600] degrees per second.
    Eigen::Vector3i gyroRaw;
    /// Raw raw value of the accelerometer, in range [0,1023] corresponding to [-4,4] g.
    Eigen::Vector3i accRaw;

    /// Parameterless constructor required for unit testing
    CM730Snapshot() {}

    CM730Snapshot(BulkReadTable const& data);

    /** Returns the gyroscope value, in hardware units, but balanced around the midpoint.
     *
     * Values may be positive or negative.
     */
    Eigen::Vector3i getBalancedGyroValue() const
    {
      return this->gyroRaw - Eigen::Vector3i(512, 512, 512);
    }
  };

  class StaticCM730State
  {
  public:
    unsigned short modelNumber;
    unsigned char firmwareVersion;
    unsigned char dynamixelId;
    unsigned int baudBPS;
    unsigned int returnDelayTimeMicroSeconds;

    /** Controls when a status packet is returned in response to an instruction.
    *
    * 0 - only for PING command
    * 1 - only for READ command
    * 2 - for all commands
    *
    * Note that a status packet is never returned for broadcast instructions.
    */
    unsigned char statusRetLevel;

    // skip dynamic addresses in the table -- they are captured in CM730Snapshot

    StaticCM730State(BulkReadTable const& data);
  };
}
