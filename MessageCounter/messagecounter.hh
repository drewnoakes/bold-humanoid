#pragma once

namespace bold
{
  class MessageCounter
  {
  public:
    MessageCounter()
      : d_gameControllerMessageCount(0),
        d_ignoredMessageCount(0),
        d_sentTeamMessageCount(0),
        d_receivedTeamMessageCount(0),
        d_sentDrawbridgeMessageCount(0)
    {
      updateStateObject();
    }

    void notifyReceivedGameControllerMessage() { d_gameControllerMessageCount++; }
    void notifyIgnoringUnrecognisedMessage() { d_ignoredMessageCount++; }
    void notifySendingTeamMessage() { d_sentTeamMessageCount++; }
    void notifyReceivedTeamMessage() { d_receivedTeamMessageCount++; }
    void notifySendingDrawbridgeMessage() { d_sentDrawbridgeMessageCount++; }

    void updateStateObject();

  private:
    unsigned d_gameControllerMessageCount;
    unsigned d_ignoredMessageCount;
    unsigned d_sentTeamMessageCount;
    unsigned d_receivedTeamMessageCount;
    unsigned d_sentDrawbridgeMessageCount;
  };
}
