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
    SequentialTimer(std::string ns)
    : d_eventTimings(std::make_shared<std::vector<EventTiming>>()),
      d_ns(ns)
    {}
    
    void start()
    {
      d_last = Clock::getTimestamp();
    }
    
    Clock::Timestamp timeEvent(std::string const& eventName)
    {
      auto timeSeconds = Clock::getSeconds(d_last);
      d_eventTimings->push_back(EventTiming(timeSeconds, eventName));
      return Clock::getTimestamp();
    }
    
    std::shared_ptr<std::vector<EventTiming>> flush()
    {
      auto old = d_eventTimings;
      d_eventTimings = std::make_shared<std::vector<EventTiming>>();
      return old;
    }

  private:
    std::shared_ptr<std::vector<EventTiming>> d_eventTimings;
    std::string d_ns;
    Clock::Timestamp d_last;
  };

  class Debugger
  {
  public:
    Debugger();

    //
    // Event timings
    //
    std::shared_ptr<SequentialTimer> getThinkTimer() const { d_thinkTimer->start(); return d_thinkTimer; }
    std::shared_ptr<SequentialTimer> getMotionTimer() const { d_thinkTimer->start(); return d_motionTimer; }

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
    
    std::shared_ptr<SequentialTimer> d_thinkTimer;
    std::shared_ptr<SequentialTimer> d_motionTimer;

    int d_lastLedFlags;
    int d_lastEyeInt;
    int d_lastHeadInt;
    unsigned d_gameControllerMessageCount;
    unsigned d_ignoredMessageCount;

    Colour::bgr d_eyeColour;
    Colour::bgr d_headColour;
  };
}
