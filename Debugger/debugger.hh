#pragma once

#include <memory>

namespace bold
{
  class Agent;
  class BehaviourControl;
  class DebugControl;

  // TODO this class's name suggests something grander than its reality

  class Debugger
  {
  public:
    Debugger(Agent* agent, std::shared_ptr<BehaviourControl> behaviourControl, std::shared_ptr<DebugControl> debugControl);

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

    bool d_showDazzle;
    unsigned d_gameControllerMessageCount;
    unsigned d_ignoredMessageCount;
    unsigned d_sentTeamMessageCount;
    unsigned d_receivedTeamMessageCount;
    unsigned d_sentDrawbridgeMessageCount;
  };
}
