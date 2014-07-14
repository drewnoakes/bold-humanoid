#pragma once

#include <memory>

namespace bold
{
  class Agent;
  class BehaviourControl;
  class ButtonObserver;
  class ButtonTracker;
  class DebugControl;
  class Voice;

  // TODO this class's name suggests something grander than its reality

  class Debugger
  {
  public:
    Debugger(
      Agent* agent,
      std::shared_ptr<BehaviourControl> behaviourControl,
      std::shared_ptr<DebugControl> debugControl,
      std::shared_ptr<Voice> voice,
      std::shared_ptr<ButtonObserver> buttonObserver);

    void notifyReceivedGameControllerMessage() { d_gameControllerMessageCount++; }
    void notifyIgnoringUnrecognisedMessage() { d_ignoredMessageCount++; }
    void notifySendingTeamMessage() { d_sentTeamMessageCount++; }
    void notifyReceivedTeamMessage() { d_receivedTeamMessageCount++; }
    void notifySendingDrawbridgeMessage() { d_sentDrawbridgeMessageCount++; }

    void showDazzle(bool showDazzle) { d_showDazzle = showDazzle; }

    /** Update DebugState. Called at the end of the think cycle. */
    void update();

  private:
    Agent* d_agent;
    std::shared_ptr<BehaviourControl> d_behaviourControl;
    std::shared_ptr<DebugControl> d_debugControl;
    std::shared_ptr<Voice> d_voice;
    std::shared_ptr<ButtonTracker> d_leftButtonTracker;

    bool d_showDazzle;
    unsigned d_gameControllerMessageCount;
    unsigned d_ignoredMessageCount;
    unsigned d_sentTeamMessageCount;
    unsigned d_receivedTeamMessageCount;
    unsigned d_sentDrawbridgeMessageCount;
  };
}
