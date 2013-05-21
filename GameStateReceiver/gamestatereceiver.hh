#pragma once

#define GAMECONTROLLER_PORT 3838

#include <memory>

#include "../StateObject/GameState/gamestate.hh"
#include "../Configurable/configurable.hh"

namespace bold
{
  class Debugger;

  class GameStateReceiver : public Configurable
  {
  public:
    GameStateReceiver(std::shared_ptr<Debugger> debugger);

    std::shared_ptr<GameState> receive();

  private:
    int d_port;
    int d_socket;
    bool d_receivedAnything;
    std::shared_ptr<Debugger> d_debugger;
  };
}
