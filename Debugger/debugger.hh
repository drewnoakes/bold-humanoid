#pragma once

#include <vector>
#include <memory>

#include "../Colour/colour.hh"
#include "../Clock/clock.hh"

namespace robotis
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

    //
    // Event timings
    //

    Clock::Timestamp timeEvent(Clock::Timestamp const& startedAt, std::string const& eventName);

    void addEventTiming(EventTiming const& eventTiming);

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
    void update(std::shared_ptr<robotis::CM730> cm730);

  private:
    void showEyeColour(Colour::bgr const& colour) { d_eyeColour = colour; }
    void showHeadColour(Colour::bgr const& colour) { d_headColour = colour; }

    int d_lastLedFlags;
    int d_lastEyeInt;
    int d_lastHeadInt;
    std::shared_ptr<std::vector<EventTiming>> d_eventTimings;
    unsigned d_gameControllerMessageCount;
    unsigned d_ignoredMessageCount;

    Colour::bgr d_eyeColour;
    Colour::bgr d_headColour;
  };
}
