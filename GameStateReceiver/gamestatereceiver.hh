#pragma once

#define GAMECONTROLLER_PORT 3838

#include <memory>
#include <set>

#include "../StateObject/GameState/gamestate.hh"

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

  class GameStateReceiver
  {
  public:
    GameStateReceiver(std::shared_ptr<Debugger> debugger, Agent* agent);

    void receive();

  private:
    void processGameControllerInfoMessage(char const* data);

    std::shared_ptr<UDPSocket> d_socket;
    std::shared_ptr<Debugger> d_debugger;
    std::set<uint32> d_observedVersionNumbers;
    std::set<uint8> d_observedTeamNumbers;
    Agent* d_agent;
    bool d_receivedAnything;
  };
}
