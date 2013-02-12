#ifndef BOLD_DEBUGGER_HH
#define BOLD_DEBUGGER_HH

#define LED_RED   0x01;
#define LED_BLUE  0x02;
#define LED_GREEN 0x04;

#include <LinuxDARwIn.h>
#include <LinuxCM730.h>
#include "../GameController/RoboCupGameControlData.h"
#include "../DataStreamer/datastreamer.hh"

namespace bold
{
  class Debugger
  {
  public:
    typedef unsigned long long timestamp_t;

  private:
    bool d_isBallObserved;
    bool d_isTwoGoalPostsObserved;
    int d_lastLEDValue;

  public:
    Debugger();

    static const timestamp_t getTimestamp();

    static const double getSeconds(timestamp_t const& startedAt);

    static const double printTime(timestamp_t const& startedAt, std::string const& description);

    void timeThinkCycle(timestamp_t const& startedAt);
    void timeImageProcessing(timestamp_t const& startedAt);
    void timeSubBoardRead(timestamp_t const& startedAt);
    void timeImageCapture(timestamp_t const& startedAt);

    void setIsBallObserved(bool const& isBallObserved);
    void setGoalObservationCount(int const& goalObservationCount);
    void setGameControlData(RoboCupGameControlData const& gameControlData);

    void update(Robot::CM730& cm730);
  };
}

#endif
