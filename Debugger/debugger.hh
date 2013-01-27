#ifndef BOLD_DEBUGGER_HH
#define BOLD_DEBUGGER_HH

#define LED_RED   0x01;
#define LED_BLUE  0x02;
#define LED_GREEN 0x04;

#include <LinuxDARwIn.h>
#include <LinuxCM730.h>

namespace bold
{
  class Debugger
  {
  public:
    typedef unsigned long long timestamp_t;

  private:
    bool d_isBallObserved;
    bool d_isImageProcessingSlow;
    int d_lastLEDValue;
    double d_imageProcessingThresholdMillis;

    static double getSeconds(timestamp_t startedAt);

  public:
    Debugger()
    : d_isBallObserved(false),
      d_isImageProcessingSlow(false),
      d_lastLEDValue(0xff),
      d_imageProcessingThresholdMillis(25.0)
    {}

    static timestamp_t getTimestamp();
    
    static void printTime(timestamp_t startedAt, std::string const& description);

    void timeImageProcessing(timestamp_t startedAt);
    void timeImageCapture(timestamp_t startedAt);

    void setIsBallObserved(bool isBallObserved);

    void update(Robot::CM730& cm730);
  };
}

#endif
