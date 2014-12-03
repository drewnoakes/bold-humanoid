#include "gamestatereceiver.hh"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "../Agent/agent.hh"
#include "../MessageCounter/messagecounter.hh"
#include "../State/state.hh"
#include "../StateObject/GameState/gamestate.hh"
#include "../UDPSocket/udpsocket.hh"
#include "../Voice/voice.hh"

using namespace bold;
using namespace robocup;
using namespace std;

GameStateReceiver::GameStateReceiver(shared_ptr<MessageCounter> messageCounter, shared_ptr<Voice> voice)
  : d_messageCounter(messageCounter),
    d_voice(voice),
    d_sendResponseMessages(Config::getSetting<bool>("game-controller.send-response-messages")),
    d_gameControllerPort(Config::getStaticValue<int>("game-controller.tcp-port")),
    d_receivedInfoMessageRecently(false)
{
  d_socket = make_shared<UDPSocket>();
  d_socket->setBlocking(false);
  d_socket->bind(d_gameControllerPort);

  log::info("GameStateReceiver::GameStateReceiver") << "Listening on UDP port " << d_gameControllerPort;
}

void GameStateReceiver::receive()
{
  static constexpr uint MaxMessageSize = GameStateMessage::SIZE; //max(GameStateMessage::SIZE, RobotStatusMessage::SIZE);

  static set<uint32> ignoredHeaders;
  static uint8 teamNumber = static_cast<uint8>(Config::getStaticValue<int>("team-number"));
  static uint8 uniformNumber = static_cast<uint8>(Config::getStaticValue<int>("uniform-number"));

  // Reuse data buffer. Makes this method unthreadsafe.
  static char data[MaxMessageSize + 1];

  // Process incoming game controller messages
  sockaddr_in fromAddress = {};
  int fromAddressLength = sizeof(sockaddr_in);
  bool received = false;

  auto logBadSize = [this](int bytesRead)
  {
    static set<int> ignoredLengths;
    if (ignoredLengths.find(bytesRead) == ignoredLengths.end())
    {
      ignoredLengths.insert(bytesRead);
      log::warning("GameStateReceiver::receive") << "First game controller message with invalid size (seen " << bytesRead << ")";
    }
    d_messageCounter->notifyIgnoringUnrecognisedMessage();
  };

  auto logBadVersion = [this](uint8 observed, uint8 expected)
  {
    static set<pair<uint8,uint8>> ignoredVersions;
    auto key = make_pair(observed, expected);
    if (ignoredVersions.find(key) == ignoredVersions.end())
    {
      ignoredVersions.insert(key);
      log::warning("GameStateReceiver::receive") << "First game controller message with wrong version (seen " << (int)observed << " but expecting " << (int)expected << ")";
    }
    d_messageCounter->notifyIgnoringUnrecognisedMessage();
  };

  auto logBadLeague = [this](League observed, League expected)
  {
    static set<pair<League,League>> ignoredLeagues;
    auto key = make_pair(observed, expected);
    if (ignoredLeagues.find(key) == ignoredLeagues.end())
    {
      ignoredLeagues.insert(key);
      log::warning("GameStateReceiver::receive") << "First game controller message with wrong league (seen " << getLeagueName(observed) << " but expecting " << getLeagueName(expected) << ")";
    }
    d_messageCounter->notifyIgnoringUnrecognisedMessage();
  };

  // Process all pending messages, looping until done
  while (true)
  {
    // Read message, allowing for one extra byte to be read (so that messages which are too large can be ignored)
    int bytesRead = d_socket->receiveFrom(data, MaxMessageSize + 1, &fromAddress, &fromAddressLength);

    // Returns zero bytes when no message available (non-blocking)
    // Returns -1 when an error occurred. UDPSocket logs the error.
    if (bytesRead <= 0)
      break;

    received = true;

    // The start of both GameState and RobotState messages has format:
    //
    // - header  (4 bytes)
    // - version (1 byte)
    //
    // For version 7 messages, the version spans 4 bytes, but only the first
    // byte need be read. So only one byte is needed to differentiate between
    // version 7 and 8 messages.
    if (bytesRead < 5)
    {
      logBadSize(bytesRead);
      break;
    }

    //
    // Determine the message type
    //

    uint32 observedHeader = (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | (data[0]);
    uint8  observedVersion = *reinterpret_cast<uint8*>(data + 4);
    League observedLeague = *reinterpret_cast<League*>(data + 5);

    if (observedHeader == GameStateMessage::HEADER_INT)
    {
      if (observedVersion != GameStateMessage::VERSION)
        logBadVersion(observedVersion, GameStateMessage::VERSION);
      else if (bytesRead != GameStateMessage::SIZE)
        logBadSize(bytesRead);
      else if (observedLeague != League::HumanoidKidSize)
        logBadLeague(observedLeague, League::HumanoidKidSize);
      else
        processGameControllerInfoMessage(data);
    }
    else if (observedHeader == RobotStatusMessage::HEADER_INT)
    {
      // This is a response message
      if (observedVersion != RobotStatusMessage::VERSION)
        logBadVersion(observedVersion, RobotStatusMessage::VERSION);
      else if (bytesRead != RobotStatusMessage::SIZE)
        logBadSize(bytesRead);
      // TODO process response messages as information about teammates or opponents
//    else
//      processGameControllerResponseMessage(data);
    }
    else
    {
      // Not an info message, nor a pong message
      if (ignoredHeaders.find(observedHeader) == ignoredHeaders.end())
      {
        ignoredHeaders.insert(observedHeader);
        log::warning("GameStateReceiver::receive") << "First game controller message with unexpected header '" << string(reinterpret_cast<char*>(data), 4) << "' (0x" << hex << observedHeader << dec << ") seen";
      }

      d_messageCounter->notifyIgnoringUnrecognisedMessage();
    }
  }

  //
  // Send response message to Game Controller
  //

  if (received && d_sendResponseMessages->getValue())
  {
    // Send a response to the game controller (the sender), stating we're alive and well
    ASSERT(fromAddress.sin_family == AF_INET);
    fromAddress.sin_port = htons(d_gameControllerPort);
    d_socket->setTarget(fromAddress);

    RobotStatusMessage response(teamNumber, uniformNumber, RobotStatusMessageType::ALIVE);

    if (!d_socket->send(reinterpret_cast<char*>(&response), sizeof(RobotStatusMessage)))
      log::warning("GameStateReceiver::receive") << "Failed sending status response message to game controller";
  }

  //
  // Announce if we lose the game controller
  //

  if (d_receivedInfoMessageRecently)
  {
    const double silenceThresholdSeconds = 5;

    auto secondsOfSilence = Clock::getSecondsSince(d_lastReceivedInfoMessageAt);
    if (secondsOfSilence > silenceThresholdSeconds)
    {
      d_voice->say("Lost game controller");
      log::warning("GameStateReceiver::receive") << "No game controller message received for " << silenceThresholdSeconds << " seconds";
      d_receivedInfoMessageRecently = false;
    }
  }

  //
  // Forget the last game controller state if it's very old
  //

  const double forgetGameControllerStateAfterSeconds = 15;

  if (State::get<GameState>() && Clock::getSecondsSince(d_lastReceivedInfoMessageAt) > forgetGameControllerStateAfterSeconds)
  {
    d_voice->say("Forgetting game controller");
    log::warning("GameStateReceiver::receive") << "No game controller message received for " << forgetGameControllerStateAfterSeconds << " seconds, so clearing previous state";
    State::set<GameState>(nullptr);
  }
}

void GameStateReceiver::processGameControllerInfoMessage(char const* data)
{
  static set<uint32> observedVersionNumbers;
  static set<uint8> ignoredTeamNumbers;
  static set<uint8> observedOpponentTeamNumbers;

  static int teamNumber = Config::getStaticValue<int>("team-number");

  auto gameState = make_shared<GameState const>(data);

  ASSERT(gameState->getVersion() == GameStateMessage::VERSION);

  // Track the other team numbers we see, and log them as new ones arrive

  uint8 teamNumber1 = gameState->getTeam1().getTeamNumber();
  uint8 teamNumber2 = gameState->getTeam2().getTeamNumber();

  bool areWeTeam1 = teamNumber1 == teamNumber;
  bool areWeTeam2 = teamNumber2 == teamNumber;

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
        << " when our team number is " << teamNumber;
    }

    d_messageCounter->notifyIgnoringUnrecognisedMessage();
    return;
  }

  uint8 otherTeamNumber = areWeTeam1 ? teamNumber2 : teamNumber1;

  if (observedOpponentTeamNumbers.find(otherTeamNumber) == observedOpponentTeamNumbers.end())
  {
    log::info("GameStateReceiver::receive") << "Seen first game controller message for our team and team number " << (int)otherTeamNumber;
    observedOpponentTeamNumbers.insert(otherTeamNumber);
  }

  d_messageCounter->notifyReceivedGameControllerMessage();

  if (!d_receivedInfoMessageRecently)
  {
    log::info("GameStateReceiver::processGameControllerInfoMessage") << "Connection with game controller established";
    d_voice->say("Found game controller");
    d_receivedInfoMessageRecently = true;
  }

  d_lastReceivedInfoMessageAt = Clock::getTimestamp();

  State::set(gameState);
}
