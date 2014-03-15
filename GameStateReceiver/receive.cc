#include "gamestatereceiver.ih"

#include "../State/state.hh"
#include "../StateObject/GameState/gamestate.hh"

#define GAMECONTROLLER_STRUCT_HEADER "RGme"
#define GAMECONTROLLER_STRUCT_VERSION 7

#define GAMECONTROLLER_RETURN_STRUCT_HEADER "RGrt"
#define GAMECONTROLLER_RETURN_STRUCT_VERSION 1

void GameStateReceiver::receive()
{
  static constexpr uint MaxMessageSize = GameState::InfoSizeBytes; //max(GameState::InfoSizeBytes, GameState::PongSizeBytes);

  // Reuse data buffer. Makes this method unthreadsafe.
  static char data[MaxMessageSize];

  // Process incoming game controller messages
  sockaddr_in fromAddress = {};
  int fromAddressLength = sizeof(sockaddr_in);
  bool received = false;

  // Process all pending messages, looping until done
  while (true)
  {
    // Read message, allowing for one extra byte to be read (so that messages which are too large can be ignored)
    auto bytesRead = d_socket->receiveFrom(data, MaxMessageSize + 1, &fromAddress, &fromAddressLength);

    // Returns zero bytes when no message available (non-blocking)
    // Returns -1 when an error occured. UDPSocket logs the error.
    if (bytesRead <= 0)
      break;

    received = true;

    //
    // Determine the message type
    //

    if (bytesRead == GameState::InfoSizeBytes && memcmp(data, GAMECONTROLLER_STRUCT_HEADER, sizeof(GAMECONTROLLER_STRUCT_HEADER) - 1) == 0)
    {
      processGameControllerInfoMessage(data);
    }
    else if (bytesRead == GameState::PongSizeBytes && memcmp(data, GAMECONTROLLER_RETURN_STRUCT_HEADER, sizeof(GAMECONTROLLER_RETURN_STRUCT_HEADER) - 1) == 0)
    {
      // This is a response message
//       processGameControllerResponseMessage(data);
    }
    else
    {
      // Not a GC message, nor a status message
      log::warning("GameStateReceiver::receive") << "Ignoring game controller message with unexpected header";
      d_debugger->notifyIgnoringUnrecognisedMessage();
    }
  }

  //
  // Send response message to Game Controller
  //

  if (received && d_sendResponseMessages->getValue())
  {
    // Send a response to the game controller (the sender), stating we're alive and well
    cout << (int)fromAddress.sin_family << endl;
    assert(fromAddress.sin_family == AF_INET);
    d_socket->setTarget(fromAddress);

    RoboCupGameControlReturnData response;
    memcpy(&response.header, GAMECONTROLLER_RETURN_STRUCT_HEADER, sizeof(response.header));
    response.version = GAMECONTROLLER_RETURN_STRUCT_VERSION;
    response.teamNumber = (uint16)d_agent->getTeamNumber();
    response.uniformNumber = (uint16)d_agent->getUniformNumber();
    response.message = (int)GameControllerResponseMessage::ALIVE;

    if (!d_socket->send(reinterpret_cast<char*>(&response), sizeof(RoboCupGameControlReturnData)))
      log::warning("GameStateReceiver::receive") << "Failed sending status response message to game controller";
  }
}

void GameStateReceiver::processGameControllerInfoMessage(char const* data)
{
    auto gameState = make_shared<GameState const>(data);

    // Verify the version of the message
    auto version = gameState->getVersion();
    if (version != GAMECONTROLLER_STRUCT_VERSION)
    {
      if (d_observedVersionNumbers.find(version) == d_observedVersionNumbers.end())
      {
        log::warning("GameStateReceiver::receive") << "First game controller message with unexpected version " << version << " seen";
        d_observedVersionNumbers.insert(version);
      }
      d_debugger->notifyIgnoringUnrecognisedMessage();
      return;
    }

    // Track the other team numbers we see, and log them as new ones arrive

    bool areWeTeam1 = gameState->teamInfo1().getTeamNumber() == d_agent->getTeamNumber();
    bool areWeTeam2 = gameState->teamInfo2().getTeamNumber() == d_agent->getTeamNumber();

    // Verify that we're one of the teams mentioned in the message
    if (!areWeTeam1 && !areWeTeam2)
    {
      log::warning("GameStateReceiver::receive") << "Ignoring game controller message for incorrect team numbers " << (int)gameState->teamInfo1().getTeamNumber() << " and " << (int)gameState->teamInfo2().getTeamNumber() << " when our team number is " << d_agent->getTeamNumber();
      d_debugger->notifyIgnoringUnrecognisedMessage();
      return;
    }

    uint8 otherTeamNumber = areWeTeam1
      ? gameState->teamInfo2().getTeamNumber()
      : gameState->teamInfo1().getTeamNumber();

    if (d_observedTeamNumbers.find(otherTeamNumber) == d_observedTeamNumbers.end())
    {
      log::info("GameStateReceiver::receive") << "Seen first game controller message for our team and team number " << otherTeamNumber;
      d_observedTeamNumbers.insert(otherTeamNumber);
    }

    d_debugger->notifyReceivedGameControllerMessage();

    State::set(gameState);
}
