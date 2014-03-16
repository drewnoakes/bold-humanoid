#pragma once

#include <memory>
#include <set>

#include "../Clock/clock.hh"
#include "../StateObject/GameState/gamestate.hh"

namespace bold
{
  class Agent;
  class Debugger;
  template<typename> class Setting;
  class UDPSocket;
  class Voice;

  /// Model of the RoboCupGameControlReturnData struct (version 2)
  struct RoboCupGameControlReturnData
  {
    RoboCupGameControlReturnData(uint8 teamNumber, uint8 uniformNumber, GameControllerResponseMessage message)
    {
      memcpy(&header, HEADER, sizeof(header));
      version = VERSION;

      this->teamNumber = teamNumber;
      this->uniformNumber = uniformNumber;
      this->message = static_cast<uint8>(message);
    }

    char header[4];
    uint8 version;
    uint8 teamNumber;
    uint8 uniformNumber;
    uint8 message;

    static constexpr char const* HEADER = "RGrt";
    static constexpr uint32 HEADER_INT = 0x74724752;
    static constexpr uint8 VERSION = 2;
    static constexpr uint8 SIZE = 8;
  };

  enum class GameControllerResponseMessage : uint8
  {
    MANUAL_PENALISE = 0,
    MANUAL_UNPENALISE = 1,
    ALIVE = 2
  };

  class GameStateReceiver
  {
  public:
    GameStateReceiver(std::shared_ptr<Debugger> debugger, std::shared_ptr<Voice> voice);

    void receive();

  private:
    void processGameControllerInfoMessage(char const* data);

    std::shared_ptr<Debugger> d_debugger;
    std::shared_ptr<UDPSocket> d_socket;
    std::shared_ptr<Voice> d_voice;
    Setting<bool>* d_sendResponseMessages;
    int d_gameControllerPort;
    bool d_receivedInfoMessageRecently;
    Clock::Timestamp d_lastReceivedInfoMessageAt;
  };
}
