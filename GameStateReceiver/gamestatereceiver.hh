#pragma once

#define GAMECONTROLLER_PORT 3838

#include <memory>

#include "../StateObject/GameState/gamestate.hh"
#include "../Configurable/configurable.hh"

namespace bold
{
  class Agent;
  class Debugger;

  class GameStateReceiver : public Configurable
  {
  public:
    GameStateReceiver(std::shared_ptr<Debugger> debugger, Agent* agent);
    ~GameStateReceiver();

    std::shared_ptr<GameState> receive();

  private:
    int d_port;
    int d_socket;
    Agent* d_agent;
    bool d_receivedAnything;
    std::shared_ptr<Debugger> d_debugger;
  };
}
