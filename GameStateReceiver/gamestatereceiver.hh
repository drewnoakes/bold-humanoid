#pragma once

#define GAMECONTROLLER_PORT 3838

#include <memory>

#include "../StateObject/GameState/gamestate.hh"
#include "../Configurable/configurable.hh"

namespace bold
{
  class UDPSocket;
  class Agent;
  class Debugger;

  struct RoboCupGameControlReturnData
  {
    char header[4];
    uint32 version;
    uint16 teamNumber;
    uint16 uniformNumber;
    uint32 message;
  };

  enum class GameControllerResponseMessage : uint32
  {
    PENALISE = 0,
    UNPENALISE = 1,
    ALIVE = 2
  };

  class GameStateReceiver : public Configurable
  {
  public:
    GameStateReceiver(std::shared_ptr<Debugger> debugger, Agent* agent);

    std::shared_ptr<GameState> receive();

  private:
    std::shared_ptr<UDPSocket> d_socket;
    std::shared_ptr<Debugger> d_debugger;
    Agent* d_agent;
    bool d_receivedAnything;
  };
}
