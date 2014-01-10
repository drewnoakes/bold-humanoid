#pragma once

#include <memory>
#include <iostream>

#include "../Colour/colour.hh"

namespace bold
{
  class CM730;
  class DebugControl;

  // TODO this class's name suggests something grander than its reality

  class Debugger
  {
  public:
    Debugger(std::shared_ptr<DebugControl> debugControl);

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

    void showExitingAgent();
    void showExitedAgent();

    /**
     * Update the debugger at the end of the think cycle.
     */
    void update();

  private:
    std::shared_ptr<DebugControl> d_debugControl;
    unsigned d_gameControllerMessageCount;
    unsigned d_ignoredMessageCount;
    unsigned d_sentTeamMessageCount;
    unsigned d_receivedTeamMessageCount;
    bool d_seenGameControllerMessageYet;

    Colour::bgr d_eyeColour;
    Colour::bgr d_foreheadColour;
  };
}
