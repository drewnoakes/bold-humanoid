#ifndef BOLD_GAME_STATE_HH
#define BOLD_GAME_STATE_HH

#include "../../GameController/RoboCupGameControlData.h"
#include "../stateobject.hh"
#include <stdexcept>

namespace bold
{
  enum class PlayMode : unsigned char
  {
    INITIAL = 0,
    READY = 1,
    SET = 2,
    PLAYING = 3,
    FINISHED = 4
  };

  // TODO can we merge RoboCupGameControlData into GameState?

  class GameState : public StateObject
  {
  public:
    GameState(RoboCupGameControlData const& gameControlData)
    : StateObject("Game")
    {
      d_secondsRemaining = gameControlData.secsRemaining;
      d_playMode = (PlayMode)gameControlData.state;
    }

    int getSecondsRemaining() const { return d_secondsRemaining; }
    PlayMode getPlayMode() const { return d_playMode; }
    std::string getPlayModeString() const
    {
      switch (d_playMode)
      {
        case PlayMode::INITIAL:
          return "Initial";
        case PlayMode::READY:
          return "Ready";
        case PlayMode::SET:
          return "Set";
        case PlayMode::PLAYING:
          return "Playing";
        case PlayMode::FINISHED:
          return "Finished";
        default:
          throw new std::runtime_error("Unsupported PlayMode enum value.");
      }
    }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    int d_secondsRemaining;
    PlayMode d_playMode;
  };
}

#endif