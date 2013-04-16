#ifndef BOLD_GAME_STATE_HH
#define BOLD_GAME_STATE_HH

#include "../stateobject.hh"

#include <stdexcept>

namespace bold
{
  #define MAX_NUM_PLAYERS 11

  typedef unsigned char  uint8;
  typedef unsigned short uint16;
  typedef unsigned int   uint32;

  enum class PlayMode : uint8
  {
    INITIAL = 0,
    READY = 1,
    SET = 2,
    PLAYING = 3,
    FINISHED = 4
  };

  enum class ExtraState : uint8
  {
    NORMAL = 0,
    PENALTYSHOOT = 1,
    OVERTIME = 2
  };

  enum class PenaltyType : uint16
  {
    NONE = 0,
    BALL_MANIPULATION = 1,
    PHYSICAL_CONTACT = 2,
    ILLEGAL_ATTACK = 3,
    ILLEGAL_DEFENSE = 4,
    REQUEST_FOR_PICKUP = 5,
    REQUEST_FOR_SERVICE = 6,
    REQUEST_FOR_PICKUP_2_SERVICE = 7,
    MANUAL = 15
  };

  class PlayerInfo
  {
  private:
    // FIELDS DESERIALISED FROM MEMORY -- DO NOT CHANGE
    PenaltyType d_penaltyType;    // penalty state of the player
    uint16 d_secsTillUnpenalised; // estimate of time till unpenalised
  };

  class TeamInfo
  {
  private:
    // FIELDS DESERIALISED FROM MEMORY -- DO NOT CHANGE
    uint8 d_teamNumber;          // unique team number
    uint8 d_teamColour;          // colour of the team
    uint8 d_goalColour;          // colour of the goal
    uint8 d_score;               // team's score
    PlayerInfo d_players[MAX_NUM_PLAYERS];       // the team's players
  };

  class GameState : public StateObject
  {
  public:
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
    // FIELDS DESERIALISED FROM MEMORY -- DO NOT CHANGE
    char d_header[4];          // header to identify the structure
    uint32 d_version;          // version of the data structure
    uint8 d_playersPerTeam;    // The number of players on a team
    PlayMode d_playMode;       // state of the game (STATE_READY, STATE_PLAYING, etc)
    uint8 d_isFirstHalf;       // 1 = game in first half, 0 otherwise
    uint8 d_kickOffTeam;       // the next team to kick off
    ExtraState d_secondaryState; // Extra state information - (STATE2_NORMAL, STATE2_PENALTYSHOOT, etc)
    uint8 d_dropInTeam;        // team that caused last drop in
    uint16 d_dropInTime;       // number of seconds passed since the last drop in.  -1 before first dropin
    uint32 d_secondsRemaining; // estimate of number of seconds remaining in the half
    TeamInfo d_teams[2];
  };
}

#endif