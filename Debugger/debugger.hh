#ifndef BOLD_DEBUGGER_HH
#define BOLD_DEBUGGER_HH

#include <vector>
#include <memory>

#include "../Colour/colour.hh"
#include "../Clock/clock.hh"

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
    Debugger();

    Clock::Timestamp timeEvent(Clock::Timestamp const& startedAt, std::string const& eventName);

    void addEventTiming(EventTiming const& eventTiming);

    std::vector<EventTiming> getTimings() const
    {
      return d_eventTimings;
    }

    void clearTimings()
    {
      d_eventTimings.clear();
    }

    void showEyeColour(Colour::bgr const& colour) { d_eyeColour = colour; }

    void showHeadColour(Colour::bgr const& colour) { d_headColour = colour; }

    void showReady();

    void showSet();

    void showPlaying();

    void showPenalized();

    void showPaused();

    /**
     * Update the debugger at the end of the think cycle.
     * Currently only updates LEDs.
     */
    void update(std::shared_ptr<Robot::CM730> cm730);

  private:
    int d_lastLedFlags;
    int d_lastEyeInt;
    int d_lastHeadInt;
    std::vector<EventTiming> d_eventTimings;

    Colour::bgr d_eyeColour;
    Colour::bgr d_headColour;
  };
}

#endif
