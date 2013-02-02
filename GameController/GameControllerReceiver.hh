#ifndef BOLD_GAME_CONTROLLER_RECEIVER_HH
#define BOLD_GAME_CONTROLLER_RECEIVER_HH

#include "RoboCupGameControlData.h"

namespace bold
{
  class GameControllerReceiver
  {
  private:
    const int d_port;
    int d_socket;
    bool d_receivedAnything;
    bool d_init;

  public:
    GameControllerReceiver(int port = GAMECONTROLLER_PORT)
    : d_port(port),
      d_socket(-1),
      d_receivedAnything(false),
      d_init(false)
    {}

    bool receive(struct RoboCupGameControlData* const gameControlData);
  };
}

#endif