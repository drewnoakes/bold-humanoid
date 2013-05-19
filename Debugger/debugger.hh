#pragma once

#include <vector>
#include <memory>
#include <sigc++/sigc++.h>

#include "../Colour/colour.hh"
#include "../Clock/clock.hh"

namespace bold
{
  class CM730;

  typedef std::pair<double, std::string> EventTiming;
  
  class SequentialTimer
  {
  public:
    SequentialTimer()
    : d_eventTimings(std::make_shared<std::vector<EventTiming>>()),
      d_last(Clock::getTimestamp()),
      d_flushed(false)
    {}
    
    void timeEvent(std::string const& eventName)
    {
      assert(!d_flushed);
      auto now = Clock::getTimestamp();
      auto timeMillis = Clock::timeStampToMillis(now - d_last);
      d_eventTimings->push_back(EventTiming(timeMillis, eventName));
      d_last = now;
    }

    // TODO rename 'getTimings'
    std::shared_ptr<std::vector<EventTiming>> flush()
    {
      d_flushed = true;
      return d_eventTimings;
    }

  private:
    std::shared_ptr<std::vector<EventTiming>> d_eventTimings;
    Clock::Timestamp d_last;
    bool d_flushed;
  };

  class Debugger
  {
  public:
    Debugger();

    //
    // UDP Message Counts
    //

    void notifyReceivedGameControllerMessage() { d_gameControllerMessageCount++; }
    void notifyIgnoringUnrecognisedMessage() { d_ignoredMessageCount++; }

    //
    // Display status
    //

    void showReady();
    void showSet();
    void showPlaying();
    void showPenalized();
    void showPaused();

    /**
     * Update the debugger at the end of the think cycle.
     * Currently only updates LEDs.
     */
    void update(std::shared_ptr<CM730> cm730);
    
  private:
    void showEyeColour(Colour::bgr const& colour) { d_eyeColour = colour; }
    void showHeadColour(Colour::bgr const& colour) { d_headColour = colour; }

    int d_lastLedFlags;
    int d_lastEyeInt;
    int d_lastHeadInt;
    unsigned d_gameControllerMessageCount;
    unsigned d_ignoredMessageCount;

    Colour::bgr d_eyeColour;
    Colour::bgr d_headColour;
  };
}
