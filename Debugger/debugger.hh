#pragma once

#include <memory>
#include <iostream>

#include "../Colour/colour.hh"

namespace bold
{
  class CM730;

  // TODO this class's name suggests something grander than its reality

  class Debugger
  {
  public:
    Debugger();

    //
    // UDP Message Counts
    //

    void notifyReceivedGameControllerMessage();
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
    bool d_seenGameControllerMessageYet;

    Colour::bgr d_eyeColour;
    Colour::bgr d_headColour;
  };
}
