#ifndef BOLD_CM730_SNAPSHOT_HH
#define BOLD_CM730_SNAPSHOT_HH

#include "../robotis/Framework/include/CM730.h"
#include <Eigen/Eigen>
#include <Eigen/Core>

class CM730Snapshot
{
private:
  unsigned short readTableWord(unsigned char* table, int addr);
  double gyroValueToDps(int value);
  double gyroValueToRps(int value);
  double accValueToGs(int value);
  Eigen::Vector3d shortToColour(unsigned short s);

public:
  unsigned short modelNumber;
  unsigned char firmwareVersion;
  unsigned char dynamixelId;
  unsigned int baudBPS;
  unsigned int returnDelayTimeMicroSeconds;
  unsigned char retLevel;
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
  unsigned char micLevelLeft;
  unsigned char micLevelRight;

  CM730Snapshot()
  {}

  bool init(Robot::CM730 cm730);
};

#endif