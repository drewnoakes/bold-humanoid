#pragma once

#include <vector>
#include <memory>
#include <deque>
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
      d_prefix(""),
      d_last(Clock::getTimestamp()),
      d_flushed(false)
    {}
    
    void timeEvent(std::string const& eventName)
    {
      assert(!d_flushed);
      auto now = Clock::getTimestamp();
      auto timeMillis = Clock::timestampToMillis(now - d_last);
      d_eventTimings->push_back(EventTiming(timeMillis, eventName));
      d_last = now;
    }

    std::shared_ptr<std::vector<EventTiming>> flush()
    {
      d_flushed = true;
      return d_eventTimings;
    }
    
    void enter(std::string name)
    {
      d_entered.push_back(std::make_pair(name, Clock::getTimestamp()));
      rebuildPrefix();
    }
    
    void exit()
    {
      assert(d_entered.size() != 0);
      auto now = Clock::getTimestamp();
      auto top = d_entered[d_entered.size() - 1];
      auto timeMillis = Clock::timestampToMillis(now - top.second);
      d_eventTimings->push_back(EventTiming(timeMillis, d_prefix));
      d_last = now;
      d_entered.pop_back();
      rebuildPrefix();
    }

  private:
    void rebuildPrefix()
    {
      std::stringstream prefix;
      bool first = true;
      for (auto const& e : d_entered)
      {
        if (first)
          first = false;
        else
          prefix << '/';
        prefix << e.first;
      }
      d_prefix = prefix.str();
    }
    
    std::shared_ptr<std::vector<EventTiming>> d_eventTimings;
    std::deque<std::pair<std::string,Clock::Timestamp>> d_entered;
    std::string d_prefix;
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
    void notifySendingTeamMessage() { d_sentTeamMessageCount++; }
    void notifyReceivedTeamMessage() { d_receivedTeamMessageCount++; }

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
    unsigned d_sentTeamMessageCount;
    unsigned d_receivedTeamMessageCount;

    Colour::bgr d_eyeColour;
    Colour::bgr d_headColour;
  };
}
