#include "gamestatereceiver.ih"

#define GAMECONTROLLER_STRUCT_HEADER "RGme"
#define GAMECONTROLLER_STRUCT_VERSION 7

#define GAMECONTROLLER_RETURN_STRUCT_HEADER "RGrt"
#define GAMECONTROLLER_RETURN_STRUCT_VERSION 1

shared_ptr<GameState> GameStateReceiver::receive()
{
  const int MAX_LENGTH = 4096; // TODO can this just be sizeof(RoboCupGameControlData)
  static char data[MAX_LENGTH];

  // Process incoming game controller messages
  sockaddr_in fromAddress;
  int fromAddressLength;

  // Process all pending messages, looping until done
  while (d_socket->receiveFrom(data, MAX_LENGTH, &fromAddress, &fromAddressLength) > 0)
  {
    // Verify header
    if (memcmp(data, GAMECONTROLLER_STRUCT_HEADER, sizeof(GAMECONTROLLER_STRUCT_HEADER) - 1) != 0)
    {
      // Silently ignore status messages from other players
      if (memcmp(data, GAMECONTROLLER_RETURN_STRUCT_HEADER, sizeof(GAMECONTROLLER_RETURN_STRUCT_HEADER) - 1) != 0)
      {
        // Not a GC message, nor a status message
        log::warning("GameStateReceiver::receive") << "Ignoring game controller message with unexpected header";
        d_debugger->notifyIgnoringUnrecognisedMessage();
      }
      continue;
    }

    auto gameState = make_shared<GameState>(data);

    // Verify the version of the message
    if (gameState->getVersion() != GAMECONTROLLER_STRUCT_VERSION)
    {
      log::warning("GameStateReceiver::receive") << "Ignoring game controller message with unexpected version";
      d_debugger->notifyIgnoringUnrecognisedMessage();
      continue;
    }

    // Verify that we're one of the teams mentioned in the message
    if (gameState->teamInfo1().getTeamNumber() != d_agent->getTeamNumber() &&
        gameState->teamInfo2().getTeamNumber() != d_agent->getTeamNumber())
    {
      log::warning("GameStateReceiver::receive") << "Ignoring game controller message for incorrect team numbers " << (int)gameState->teamInfo1().getTeamNumber() << " and " << (int)gameState->teamInfo2().getTeamNumber() << " when our team number is " << d_agent->getTeamNumber();
      d_debugger->notifyIgnoringUnrecognisedMessage();
      continue;
    }

    d_debugger->notifyReceivedGameControllerMessage();

//     // Send a response to the game controller (the sender), stating we're alive and well
//     assert(fromAddress.sa_family == AF_INET);
//     d_socket->setTarget(fromAddress);
//
//     RoboCupGameControlReturnData response;
//     memcpy(&response.header, GAMECONTROLLER_RETURN_STRUCT_HEADER, sizeof(response.header));
//     response.version = GAMECONTROLLER_RETURN_STRUCT_VERSION;
//     response.teamNumber = (uint16)d_agent->getTeamNumber();
//     response.uniformNumber = (uint16)d_agent->getUniformNumber();
//     response.message = (int)GameControllerResponseMessage::ALIVE;
//
//     if (!d_socket->send((char*)(&response), sizeof(RoboCupGameControlReturnData)))
//       log::warning("GameStateReceiver::receive") << "Failed sending status response message to game controller";

    return gameState;
  }

  return nullptr;
}
