#ifndef BOLD_CM730_SNAPSHOT_HH
#define BOLD_CM730_SNAPSHOT_HH

#include "../robotis/Framework/include/CM730.h"

//#include <Eigen/Eigen>
#include <Eigen/Core>

namespace bold
{
  class CM730Snapshot
  {
  public:
    unsigned short modelNumber;
    unsigned char firmwareVersion;
    unsigned char dynamixelId;
    unsigned int baudBPS;
    unsigned int returnDelayTimeMicroSeconds;

    /**
    * Controls when a status packet is returned.
    *
    * 0 - only for PING command
    * 1 - only for READ command
    * 2 - for all commands
    *
    * Never returned if instruction is a broadcast packet.
    */
    unsigned char statusRetLevel;
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

    bool init(Robot::CM730& cm730);

  private:
    static unsigned short readTableWord(unsigned char* table, int addr);
    static double gyroValueToDps(int value);
    static double gyroValueToRps(int value);
    static double accValueToGs(int value);
    static Eigen::Vector3d shortToColour(unsigned short s);
  };
}

#endif
