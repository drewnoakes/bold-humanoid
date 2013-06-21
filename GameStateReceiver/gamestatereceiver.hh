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
