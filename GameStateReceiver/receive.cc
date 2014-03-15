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

  static std::set<uint32> ignoredHeaders;

  // Reuse data buffer. Makes this method unthreadsafe.
  static char data[MaxMessageSize];

  // Process incoming game controller messages
  sockaddr_in fromAddress = {};
  int fromAddressLength = sizeof(sockaddr_in);
  bool received = false;

  auto logBadSize = [this](int bytesRead)
  {
    static std::set<int> ignoredLengths;
    if (ignoredLengths.find(bytesRead) == ignoredLengths.end())
    {
      ignoredLengths.insert(bytesRead);
      log::warning("GameStateReceiver::receive") << "First game controller message with wrong size seen (" << bytesRead << " but expecting " << GameState::InfoSizeBytes << ")";
    }
  };

  // Process all pending messages, looping until done
  while (true)
  {
    // Read message, allowing for one extra byte to be read (so that messages which are too large can be ignored)
    int bytesRead = d_socket->receiveFrom(data, MaxMessageSize + 1, &fromAddress, &fromAddressLength);

    // Returns zero bytes when no message available (non-blocking)
    // Returns -1 when an error occured. UDPSocket logs the error.
    if (bytesRead <= 0)
      break;

    received = true;

    // We need at least 5 bytes for the header (4) and version (1).
    // For version 7 messages, the version spans 4 bytes, but only the first
    // byte need be read. So only one byte is needed to differentiate between
    // version 7 and 8 messages.
    if (bytesRead <= 5)
    {
      logBadSize(bytesRead);
      d_debugger->notifyIgnoringUnrecognisedMessage();
      break;
    }

    //
    // Determine the message type
    //

    const char* infoHeaderChars = "RGme";
    const char* responseHeaderChars = "RGme";
    const uint32 infoHeaderInt  = *reinterpret_cast<uint32 const*>(infoHeaderChars);
    const uint32 responseHeader = *reinterpret_cast<uint32 const*>(responseHeaderChars);

    uint32 observedHeader = *reinterpret_cast<uint32*>(data);

    if (observedHeader == infoHeaderInt)
    {
      if (bytesRead != GameState::InfoSizeBytes)
      {
        logBadSize(bytesRead);
        d_debugger->notifyIgnoringUnrecognisedMessage();
      }
      else
      {
        processGameControllerInfoMessage(data);
      }
    }
    else if (observedHeader == responseHeader)
    {
      // This is a response message
      if (bytesRead != GameState::PongSizeBytes)
      {
        logBadSize(bytesRead);
        d_debugger->notifyIgnoringUnrecognisedMessage();
      }
      else
      {
        // TODO process response messages as information about teammates or opponents
//       processGameControllerResponseMessage(data);
      }
    }
    else
    {
      // Not an info message, nor a pong message
      if (ignoredHeaders.find(observedHeader) == ignoredHeaders.end())
      {
        ignoredHeaders.insert(observedHeader);
        log::warning("GameStateReceiver::receive") << "First game controller message with unexpected header '" << string(reinterpret_cast<char*>(data), 4) << "' seen";
      }

      d_debugger->notifyIgnoringUnrecognisedMessage();
    }
  }

  //
  // Send response message to Game Controller
  //

  if (received && d_sendResponseMessages->getValue())
  {
    // Send a response to the game controller (the sender), stating we're alive and well
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
    static std::set<uint32> observedVersionNumbers;
    static std::set<uint8> ignoredTeamNumbers;
    static std::set<uint8> observedOpponentTeamNumbers;

    auto gameState = make_shared<GameState const>(data);

    // Verify the version of the message
    uint32 version = gameState->getVersion();
    if (version != GAMECONTROLLER_STRUCT_VERSION)
    {
      if (observedVersionNumbers.find(version) == observedVersionNumbers.end())
      {
        log::warning("GameStateReceiver::receive") << "First game controller message with unexpected version " << version << " seen";
        observedVersionNumbers.insert(version);
      }
      d_debugger->notifyIgnoringUnrecognisedMessage();
      return;
    }

    // Track the other team numbers we see, and log them as new ones arrive

    uint8 teamNumber1 = gameState->teamInfo1().getTeamNumber();
    uint8 teamNumber2 = gameState->teamInfo2().getTeamNumber();

    bool areWeTeam1 = teamNumber1 == d_agent->getTeamNumber();
    bool areWeTeam2 = teamNumber2 == d_agent->getTeamNumber();

    // Verify that we're one of the teams mentioned in the message
    if (!areWeTeam1 && !areWeTeam2)
    {
      if (ignoredTeamNumbers.find(teamNumber1) == ignoredTeamNumbers.end() ||
          ignoredTeamNumbers.find(teamNumber2) == ignoredTeamNumbers.end())
      {
        ignoredTeamNumbers.insert(teamNumber1);
        ignoredTeamNumbers.insert(teamNumber2);

        log::warning("GameStateReceiver::receive")
          << "Ignoring game controller message for incorrect team numbers "
          << (int)teamNumber1 << " and " << (int)teamNumber2
          << " when our team number is " << d_agent->getTeamNumber();
      }

      d_debugger->notifyIgnoringUnrecognisedMessage();
      return;
    }

    uint8 otherTeamNumber = areWeTeam1 ? teamNumber2 : teamNumber1;

    if (observedOpponentTeamNumbers.find(otherTeamNumber) == observedOpponentTeamNumbers.end())
    {
      log::info("GameStateReceiver::receive") << "Seen first game controller message for our team and team number " << (int)otherTeamNumber;
      observedOpponentTeamNumbers.insert(otherTeamNumber);
    }

    d_debugger->notifyReceivedGameControllerMessage();

    State::set(gameState);
}
