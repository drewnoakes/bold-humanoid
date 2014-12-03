#include "messagecounter.hh"

#include "../State/state.hh"
#include "../StateObject/MessageCountState/messagecountstate.hh"

using namespace bold;

void MessageCounter::updateStateObject()
{
  State::make<MessageCountState>(
    d_gameControllerMessageCount,
    d_ignoredMessageCount,
    d_sentTeamMessageCount,
    d_receivedTeamMessageCount,
    d_sentDrawbridgeMessageCount
  );

  // clear accumulators for next cycle
  d_gameControllerMessageCount = 0;
  d_ignoredMessageCount = 0;
  d_sentTeamMessageCount = 0;
  d_receivedTeamMessageCount = 0;
  d_sentDrawbridgeMessageCount = 0;
}
