#ifndef BOLD_DEBUGGER_HH
#define BOLD_DEBUGGER_HH

#include <vector>

#include <LinuxDARwIn.h>
#include <LinuxCM730.h>

#define LED_RED   0x01;
#define LED_BLUE  0x02;
#define LED_GREEN 0x04;

namespace bold
{
  typedef std::pair<double,std::string> EventTiming;

  class Debugger
  {
  public:
    typedef unsigned long long timestamp_t;

  private:
    Debugger();
    int d_lastLedFlags;
    std::vector<EventTiming> d_eventTimings;

  public:
    static const timestamp_t getTimestamp();

    static const double getSeconds(timestamp_t const& startedAt);

    static const double printTime(timestamp_t const& startedAt, std::string const& description);

    timestamp_t timeEvent(timestamp_t const& startedAt, std::string const& eventName);

    void addEventTiming(EventTiming const& eventTiming);

    std::vector<EventTiming> getTimings()
    {
      return d_eventTimings;
    }

    void clearTimings()
    {
      d_eventTimings.clear();
    }

    /**
     * Update the debugger at the end of the think cycle.
     * Currently only updates LEDs.
     */
    void update(Robot::CM730& cm730);

    static Debugger& getInstance()
    {
      static Debugger instance;
      return instance;
    }
  };
}

#endif
