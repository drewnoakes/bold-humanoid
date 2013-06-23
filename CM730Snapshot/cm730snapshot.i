%{
#include <CM730Snapshot/cm730snapshot.hh>
%}

namespace bold
{
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

    Eigen::Vector3i getBalancedGyroValue() const;
  };

  class StaticCM730State
  {
  public:
    unsigned short modelNumber;
    unsigned char firmwareVersion;
    unsigned char dynamixelId;
    unsigned int baudBPS;
    unsigned int returnDelayTimeMicroSeconds;

    unsigned char statusRetLevel;
  };
}
