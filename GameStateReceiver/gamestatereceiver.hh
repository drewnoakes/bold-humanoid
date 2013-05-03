#ifndef BOLD_GAME_CONTROLLER_RECEIVER_HH
#define BOLD_GAME_CONTROLLER_RECEIVER_HH

#define GAMECONTROLLER_PORT 3838

#include <memory>
#include <minIni.h>

#include "../StateObject/GameState/gamestate.hh"

namespace bold
{
  class Debugger;

  class GameStateReceiver
  {
  public:
    GameStateReceiver(minIni const& ini, std::shared_ptr<Debugger> debugger);

    std::shared_ptr<GameState> receive();

  private:
    int d_port;
    int d_socket;
    bool d_receivedAnything;
    std::shared_ptr<Debugger> d_debugger;
  };
}

#endif