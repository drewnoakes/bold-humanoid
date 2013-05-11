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
    bool isLed2On;
    bool isLed3On;
    bool isLed4On;
    Eigen::Vector3d eyeColor;
    Eigen::Vector3d foreheadColor;
    bool isModeButtonPressed;
    bool isStartButtonPressed;
    Eigen::Vector3d gyro;
    Eigen::Vector3d acc;
    float voltage;
    Eigen::Vector3i gyroRaw;
    Eigen::Vector3i accRaw;

    CM730Snapshot(BulkReadTable const& data);
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

    unsigned char micLevelLeft;
    unsigned char micLevelRight;

    StaticCM730State(BulkReadTable const& data);
  };
}
