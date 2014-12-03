#pragma once

#include <memory>
#include <set>

#include "../Clock/clock.hh"
#include "../StateObject/GameState/gamestate.hh"

namespace bold
{
  class Agent;
  class MessageCounter;
  template<typename> class Setting;
  class UDPSocket;
  class Voice;

  class GameStateReceiver
  {
  public:
    GameStateReceiver(std::shared_ptr<MessageCounter> messageCounter, std::shared_ptr<Voice> voice);

    void receive();

  private:
    void processGameControllerInfoMessage(char const* data);

    std::shared_ptr<MessageCounter> d_messageCounter;
    std::shared_ptr<UDPSocket> d_socket;
    std::shared_ptr<Voice> d_voice;
    Setting<bool>* d_sendResponseMessages;
    int d_gameControllerPort;
    bool d_receivedInfoMessageRecently;
    Clock::Timestamp d_lastReceivedInfoMessageAt;
  };
}
