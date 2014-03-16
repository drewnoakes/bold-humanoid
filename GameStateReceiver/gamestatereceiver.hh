#pragma once

#include <memory>
#include <set>

#include "../StateObject/GameState/gamestate.hh"

namespace bold
{
  class UDPSocket;
  class Agent;
  class Debugger;
  template<typename> class Setting;

  /// Model of the RoboCupGameControlReturnData struct (version 2)
  struct RoboCupGameControlReturnData
  {
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
    GameStateReceiver(std::shared_ptr<Debugger> debugger);

    void receive();

  private:
    void processGameControllerInfoMessage(char const* data);

    std::shared_ptr<Debugger> d_debugger;
    std::shared_ptr<UDPSocket> d_socket;
    Setting<bool>* d_sendResponseMessages;
  };
}
