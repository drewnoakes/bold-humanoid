#include "gamestatereceiver.ih"

#define GAMECONTROLLER_STRUCT_HEADER "RGme"
#define GAMECONTROLLER_STRUCT_VERSION 7

shared_ptr<GameState> GameStateReceiver::receive()
{
  const int MAX_LENGTH = 4096; // TODO can this just be sizeof(RoboCupGameControlData)
  static char data[MAX_LENGTH];

  // Process incoming game controller messages
  sockaddr fromAddress;
  int fromAddressLength;

  // Loop until we get the most recent messages
  // TODO this looping may not be a good idea, in case we miss an important message
  //      there are only two a second, so it shouldn't be hard to process all of them
  // TODO use the fromAddress to send a response message stating that we're alive and well
  while (d_socket->receiveFrom(data, MAX_LENGTH, &fromAddress, &fromAddressLength) > 0)
  {
    // Verify header
    if (memcmp(data, GAMECONTROLLER_STRUCT_HEADER, sizeof(GAMECONTROLLER_STRUCT_HEADER) - 1) != 0)
    {
      cerr << "[GameStateReceiver::receive] Ignoring game controller message with unexpected header" << endl;
      d_debugger->notifyIgnoringUnrecognisedMessage();
      continue;
    }

    auto gameState = make_shared<GameState>(data);

    // Verify the version of the message
    if (gameState->getVersion() != GAMECONTROLLER_STRUCT_VERSION)
    {
      cerr << "[GameStateReceiver::receive] Ignoring game controller message with unexpected version" << endl;
      d_debugger->notifyIgnoringUnrecognisedMessage();
      continue;
    }

    // Verify that we're one of the teams mentioned in the message
    if (gameState->teamInfo1().getTeamNumber() != d_agent->getTeamNumber() &&
        gameState->teamInfo2().getTeamNumber() != d_agent->getTeamNumber())
    {
      cerr << "[GameStateReceiver::receive] Ignoring game controller message for incorrect team numbers " << (int)gameState->teamInfo1().getTeamNumber() << " and " << (int)gameState->teamInfo2().getTeamNumber() << " when our team number is " << d_agent->getTeamNumber() << endl;
      d_debugger->notifyIgnoringUnrecognisedMessage();
      continue;
    }

    d_debugger->notifyReceivedGameControllerMessage();
    return gameState;
  }

  return nullptr;
}
