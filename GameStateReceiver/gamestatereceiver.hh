#ifndef BOLD_GAME_CONTROLLER_RECEIVER_HH
#define BOLD_GAME_CONTROLLER_RECEIVER_HH

#define GAMECONTROLLER_PORT 3838

#include <memory>

#include "../StateObject/GameState/gamestate.hh"

namespace bold
{
  class GameStateReceiver
  {
  private:
    const int d_port;
    int d_socket;
    bool d_receivedAnything;
    bool d_isInitialised;

  public:
    GameStateReceiver(int port = GAMECONTROLLER_PORT)
    : d_port(port),
      d_socket(-1),
      d_receivedAnything(false),
      d_isInitialised(false)
    {}

    std::shared_ptr<GameState> receive();
  };
}

#endif