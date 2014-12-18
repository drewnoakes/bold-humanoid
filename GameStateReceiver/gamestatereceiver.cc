#include "gamestatereceiver.hh"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <set>

#include "../Agent/agent.hh"
#include "../MessageCounter/messagecounter.hh"
#include "../GameStateDecoder/gamestatedecoder.hh"
#include "../State/state.hh"
#include "../StateObject/GameState/gamestate.hh"
#include "../UDPSocket/udpsocket.hh"
#include "../Voice/voice.hh"
#include "../util/bufferwriter.hh"

using namespace bold;
using namespace std;

GameStateReceiver::GameStateReceiver(shared_ptr<MessageCounter> messageCounter, shared_ptr<Voice> voice)
  : d_messageCounter(messageCounter),
    d_voice(voice),
    d_sendResponseMessages(Config::getSetting<bool>("game-controller.send-response-messages")),
    d_gameControllerPort((uint16_t)Config::getStaticValue<int>("game-controller.tcp-port")),
    d_receivedGameStateRecently(false),
    d_activeGameStateVersion(0),
    d_maxMessageSize(0)
{
  d_socket = make_shared<UDPSocket>();
  d_socket->setBlocking(false);
  d_socket->bind(d_gameControllerPort);

  log::info("GameStateReceiver::GameStateReceiver") << "Listening on UDP port " << d_gameControllerPort;
}

void GameStateReceiver::addDecoder(unique_ptr<GameStateDecoder> decoder)
{
  ASSERT(decoder);

  log::verbose("GameStateReceiver::addDecoder") << "Adding GameState message decoder for protocol version "
    << (int)decoder->getSupportedVersion();

  d_maxMessageSize = max(d_maxMessageSize, decoder->getMessageSize());

  auto res = d_decoderByVersion.emplace(decoder->getSupportedVersion(), move(decoder));

  if (!res.second)
  {
    log::error("GameStateReceiver::addDecoder") << "Cannot register GameStateDecoder for version "
      << (int)res.first->second->getSupportedVersion() << " as an existing instance has already been registered";
    throw runtime_error("Attempt to register multiple GameStateDecoders with the same version");
  }
}

void GameStateReceiver::receive()
{
  ASSERT(!d_decoderByVersion.empty());

  static uint8_t teamNumber = static_cast<uint8_t>(Config::getStaticValue<int>("team-number"));
  static uint8_t uniformNumber = static_cast<uint8_t>(Config::getStaticValue<int>("uniform-number"));

  char data[d_maxMessageSize + 1];

  // Process incoming game controller messages
  sockaddr_in fromAddress = {};
  int fromAddressLength = sizeof(sockaddr_in);
  bool received = false;

  static constexpr uint32_t GameStateMagicNumber = 0x656d4752; // "RGme"
  static constexpr uint32_t RobotStatusMagicNumber = 0x74724752; // "RGrt"

  // Process all pending messages, looping until done
  while (true)
  {
    // Read message, allowing for one extra byte to be read (so that messages which are too large can be ignored)
    int bytesRead = d_socket->receiveFrom(data, d_maxMessageSize + 1, &fromAddress, &fromAddressLength);

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
      static set<int> ignored;
      log::warnOnce(ignored, bytesRead) << "First game controller message with invalid size (seen " << bytesRead << ")";
      d_messageCounter->notifyIgnoringUnrecognisedMessage();
      break;
    }

    //
    // Determine the message type
    //

    BufferReader reader(data);

    uint32_t magicNumber = reader.readInt32u();
    uint8_t version = reader.readInt8u();

    switch (magicNumber)
    {
      case GameStateMagicNumber:
      {
        // Take the highest available game state version
        if (version < d_activeGameStateVersion)
        {
          static set<pair<uint8_t,uint8_t>> ignored;
          log::warnOnce(ignored, make_pair(version, d_activeGameStateVersion), "GameStateReceiver")
            << "Ignoring game state message with version " << (int)version
            << " as currently only receiving version " << (int)d_activeGameStateVersion << " or above";
          continue;
        }
        d_activeGameStateVersion = version;

        auto decoder = d_decoderByVersion.find(version);

        if (decoder == d_decoderByVersion.end())
        {
          static set<uint8_t> ignored;
          log::warnOnce(ignored, version) << "Ignoring message with unsupported game state version " << (int)version;
          d_messageCounter->notifyIgnoringUnrecognisedMessage();
        }
        else
        {
          const int messageSize = decoder->second->getMessageSize();

          if (bytesRead != messageSize)
          {
            static set<int> ignored;
            log::warnOnce(ignored, bytesRead) << "Ignoring illegally sized game state message with version "
              << (int)version << ", should be " << messageSize << " bytes, not " << bytesRead;
            d_messageCounter->notifyIgnoringUnrecognisedMessage();
          }
          else
          {
            auto gameState = decoder->second->decode(reader);
            ASSERT((int)reader.pos() == messageSize);
            ASSERT(gameState->getVersion() == version);
            processGameState(gameState);
          }
        }

        break;
      }
      case RobotStatusMagicNumber:
      {
        static constexpr uint8_t RobotStatusMessageVersion = 2;
        static constexpr uint8_t RobotStatusMessageSize = 8;

        // This is a response message
        if (version != RobotStatusMessageVersion)
        {
          static set<uint8_t> ignored;
          log::warnOnce(ignored, version) << "First robot status message with wrong version (seen "
            << (int)version << " but expecting " << (int)RobotStatusMessageVersion << ")";
          d_messageCounter->notifyIgnoringUnrecognisedMessage();
        }
        else if (bytesRead != RobotStatusMessageSize)
        {
          static set<int> ignored;
          log::warnOnce(ignored, bytesRead) << "Ignoring illegally sized robot status message with version "
            << (int)version << ", should be " << RobotStatusMessageSize << " bytes, not " << bytesRead;
          d_messageCounter->notifyIgnoringUnrecognisedMessage();
        }
        // TODO process response messages as information about teammates or opponents
//      else
//        processGameControllerResponseMessage(data);
        break;
      }
      default:
      {
        static set<uint32_t> ignored;
        log::warnOnce(ignored, magicNumber) << "First game controller message with unexpected magic number '"
          << string(reinterpret_cast<char*>(&magicNumber), 4) << "' (0x" << hex << magicNumber << dec << ") seen";
        d_messageCounter->notifyIgnoringUnrecognisedMessage();
      }
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

    static constexpr size_t MessageSizeBytes = 8;
    static constexpr uint8_t MessageVersion = 2;

    char buffer[MessageSizeBytes];
    BufferWriter writer(buffer);
    writer.writeInt32u(RobotStatusMagicNumber);
    writer.writeInt8u(MessageVersion);
    writer.writeInt8u(teamNumber);
    writer.writeInt8u(uniformNumber);
    writer.writeInt8u((uint8_t)RobotStatusMessageType::ALIVE);
    ASSERT(writer.pos() == MessageSizeBytes);

    if (!d_socket->send(buffer, MessageSizeBytes))
      log::warning("GameStateReceiver::receive") << "Failed sending status response message to game controller";
  }

  //
  // Announce if we lose the game controller
  //

  if (d_receivedGameStateRecently)
  {
    const double silenceThresholdSeconds = 5;

    auto secondsOfSilence = Clock::getSecondsSince(d_gameStateReceivedAt);
    if (secondsOfSilence > silenceThresholdSeconds)
    {
      d_voice->say("Lost game controller");
      log::warning("GameStateReceiver::receive") << "No game controller message received for " << silenceThresholdSeconds << " seconds";
      d_receivedGameStateRecently = false;
      d_activeGameStateVersion = 0;
    }
  }

  //
  // Forget the last game controller state if it's very old
  //

  const double forgetGameControllerStateAfterSeconds = 15;

  if (State::get<GameState>() && Clock::getSecondsSince(d_gameStateReceivedAt) > forgetGameControllerStateAfterSeconds)
  {
    d_voice->say("Forgetting game controller");
    log::warning("GameStateReceiver::receive") << "No game controller message received for " << forgetGameControllerStateAfterSeconds << " seconds, so clearing previous state";
    State::set<GameState>(nullptr);
  }
}

void GameStateReceiver::processGameState(shared_ptr<GameState const> gameState)
{
  static set<uint32_t> observedVersionNumbers;
  static set<uint8_t> ignoredTeamNumbers;
  static set<uint8_t> observedOpponentTeamNumbers;

  static int teamNumber = Config::getStaticValue<int>("team-number");

  // Track the other team numbers we see, and log them as new ones arrive

  uint8_t teamNumber1 = gameState->getTeam1().getTeamNumber();
  uint8_t teamNumber2 = gameState->getTeam2().getTeamNumber();

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

      log::warning("GameStateReceiver::processGameState")
        << "Ignoring game controller message for incorrect team numbers "
        << (int)teamNumber1 << " and " << (int)teamNumber2
        << " when our team number is " << teamNumber;
    }

    d_messageCounter->notifyIgnoringUnrecognisedMessage();
    return;
  }

  uint8_t otherTeamNumber = areWeTeam1 ? teamNumber2 : teamNumber1;

  if (observedOpponentTeamNumbers.find(otherTeamNumber) == observedOpponentTeamNumbers.end())
  {
    log::info("GameStateReceiver::processGameState") << "Seen first game controller message for our team and team number " << (int)otherTeamNumber;
    observedOpponentTeamNumbers.insert(otherTeamNumber);
  }

  d_messageCounter->notifyReceivedGameControllerMessage();

  if (!d_receivedGameStateRecently)
  {
    log::info("GameStateReceiver::processGameState") << "Connection with game controller established";
    d_voice->say("Found game controller");
    d_receivedGameStateRecently = true;
  }

  d_gameStateReceivedAt = Clock::getTimestamp();

  State::set(gameState);
}
