#ifndef BOLD_DEBUGGER_HH
#define BOLD_DEBUGGER_HH

#include <vector>
#include <memory>

#define LED_RED   0x01;
#define LED_BLUE  0x02;
#define LED_GREEN 0x04;

namespace Robot
{
  class CM730;
}

namespace bold
{
  typedef std::pair<double,std::string> EventTiming;

  class Debugger
  {
  public:
    typedef unsigned long long timestamp_t;

    static const timestamp_t getTimestamp();

    static const double getSeconds(timestamp_t const& startedAt);

    static const double printTime(timestamp_t const& startedAt, std::string const& description);

    Debugger();

    timestamp_t timeEvent(timestamp_t const& startedAt, std::string const& eventName);

    void addEventTiming(EventTiming const& eventTiming);

    std::vector<EventTiming> getTimings() const
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
    void update(std::shared_ptr<Robot::CM730> cm730);

  private:
    int d_lastLedFlags;
    std::vector<EventTiming> d_eventTimings;
  };
}

#endif
