#ifndef BOLD_GAME_STATE_HH
#define BOLD_GAME_STATE_HH

//#include <stdio.h>
#include <sigc++/sigc++.h>
#include "../GameController/RoboCupGameControlData.h"

namespace bold
{
  enum PlayMode : unsigned char
  {
    INITIAL = 0,
    READY = 1,
    SET = 2,
    PLAYING = 3,
    FINISHED = 4
  };

  class GameState
  {
  private:
    GameState()
    : secondsRemaining(-1),
      playMode(INITIAL)
    {};

    GameState(GameState const&);
    void operator=(GameState const&);

  public:
    int secondsRemaining;
    PlayMode playMode;

    sigc::signal<void> updated;

    void update(RoboCupGameControlData const& gameControlData)
    {
      secondsRemaining = gameControlData.secsRemaining;
      playMode = (PlayMode)gameControlData.state;

      updated();
    }

    static GameState& getInstance()
    {
      static GameState instance;
      return instance;
    }
  };
}

#endif