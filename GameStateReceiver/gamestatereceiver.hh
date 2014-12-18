#pragma once

#include <cstdint>
#include <map>
#include <memory>

#include "../Clock/clock.hh"

namespace bold
{
  class GameState;
  class GameStateDecoder;
  class MessageCounter;
  template<typename> class Setting;
  class UDPSocket;
  class Voice;

  /**
  * Listens for and integrates data sent according to the game controller protocol.
   *
   * This involves two types of message:
   *
   *  - GameState
   *  - RobotStatus
   *
   * In 'normal' usage, we listen for GameState and send RobotStatus, however it is possible to listen
   * for status messages from other team mates and potentially even the other team.
   */
  class GameStateReceiver
  {
  public:
    GameStateReceiver(std::shared_ptr<MessageCounter> messageCounter, std::shared_ptr<Voice> voice);

    void addDecoder(std::unique_ptr<GameStateDecoder> decoder);

    void receive();

  private:
    void processGameState(std::shared_ptr<GameState const> gameState);

    std::map<uint8_t,std::unique_ptr<GameStateDecoder>> d_decoderByVersion;

    std::shared_ptr<MessageCounter> d_messageCounter;
    std::shared_ptr<UDPSocket> d_socket;
    std::shared_ptr<Voice> d_voice;
    Setting<bool>* d_sendResponseMessages;
    uint16_t d_gameControllerPort;

    bool d_receivedGameStateRecently;
    Clock::Timestamp d_gameStateReceivedAt;
    uint8_t d_activeGameStateVersion;
    uint d_maxMessageSize;
  };
}
