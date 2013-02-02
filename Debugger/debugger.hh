#ifndef BOLD_DEBUGGER_HH
#define BOLD_DEBUGGER_HH

#define LED_RED   0x01;
#define LED_BLUE  0x02;
#define LED_GREEN 0x04;

#include <LinuxDARwIn.h>
#include <LinuxCM730.h>
#include "../GameController/RoboCupGameControlData.h"

namespace bold
{
  class Debugger
  {
  public:
    typedef unsigned long long timestamp_t;

  private:
    bool d_isBallObserved;
    bool d_isTwoGoalPostsObserved;
    bool d_isImageProcessingSlow;
    int d_lastLEDValue;
    double d_imageProcessingThresholdMillis;

  public:
    Debugger()
    : d_isBallObserved(false),
      d_isTwoGoalPostsObserved(false),
      d_isImageProcessingSlow(false),
      d_lastLEDValue(0xff),
      d_imageProcessingThresholdMillis(25.0)
    {}

    static const timestamp_t getTimestamp();

    static const double getSeconds(timestamp_t const& startedAt);

    static void printTime(timestamp_t const& startedAt, std::string const& description);

    void timeImageProcessing(timestamp_t const& startedAt);
    void timeImageCapture(timestamp_t const& startedAt);

    void setIsBallObserved(bool const& isBallObserved);
    void setGoalObservationCount(int const& goalObservationCount);

    void update(Robot::CM730& cm730);
    void setGameControlData(RoboCupGameControlData const& gameControlData);
  };
}

#endif
