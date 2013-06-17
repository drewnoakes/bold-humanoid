#include "gamestatereceiver.ih"

#define GAMECONTROLLER_STRUCT_HEADER "RGme"
#define GAMECONTROLLER_STRUCT_VERSION 7

shared_ptr<GameState> GameStateReceiver::receive()
{
  const int MAX_LENGTH = 4096;
  static char data[MAX_LENGTH];

  // Process incoming game controller messages
  static sockaddr_in source_addr;
  socklen_t source_addr_len = sizeof(source_addr);  
  
  // Loop until we get the most recent messages
  // TODO this looping may not be a good idea, in case we miss an important message
  //      there are only two a second, so it shouldn't be hard to process all of them
  // TODO no need to have the source_addr filled in -- skip it (and test)
  while (recvfrom(d_socket, data, MAX_LENGTH, 0, (struct sockaddr*) &source_addr, &source_addr_len) > 0)
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
    if (gameState->teamInfo1().getTeamNumber() != d_ourTeamNumber &&
        gameState->teamInfo2().getTeamNumber() != d_ourTeamNumber)
    {
      cerr << "[GameStateReceiver::receive] Ignoring game controller message for incorrect team numbers " << (int)gameState->teamInfo1().getTeamNumber() << " and " << (int)gameState->teamInfo2().getTeamNumber() << " when our team number is " << d_ourTeamNumber << endl;
      d_debugger->notifyIgnoringUnrecognisedMessage();
      continue;
    }

    d_debugger->notifyReceivedGameControllerMessage();
    return gameState;
  }

  return nullptr;
}
