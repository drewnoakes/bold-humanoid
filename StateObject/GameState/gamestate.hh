#pragma once

#include "../stateobject.hh"

#include <cassert>
#include <stdexcept>
#include <vector>

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
  public:
    bool hasPenalty() const { return d_penaltyType != PenaltyType::NONE; }
    PenaltyType getPenaltyType() const { return d_penaltyType; }
    std::string getPenaltyTypeString() const
    {
      switch (d_penaltyType)
      {
        case PenaltyType::NONE:
          return "No Penalty";
        case PenaltyType::BALL_MANIPULATION:
          return "Ball Manipulation";
        case PenaltyType::PHYSICAL_CONTACT:
          return "Physical Contact";
        case PenaltyType::ILLEGAL_ATTACK:
          return "Illegal Attack";
        case PenaltyType::ILLEGAL_DEFENSE:
          return "Illegal Defense";
        case PenaltyType::REQUEST_FOR_PICKUP:
          return "Request For Pickup";
        case PenaltyType::REQUEST_FOR_SERVICE:
          return "Request For Service";
        case PenaltyType::REQUEST_FOR_PICKUP_2_SERVICE:
          return "Request For Pickup To Service";
        case PenaltyType::MANUAL:
          return "Manual";
        default:
          throw new std::runtime_error("Unsupported PenaltyType enum value.");
      }
    }
    /** estimate of time till unpenalised */
    uint16 getSecondsUntilPenaltyLifted() const { return d_secondsUntilPenaltyLifted; }

  private:
    // FIELDS DESERIALISED FROM MEMORY -- DO NOT CHANGE
    PenaltyType d_penaltyType;
    uint16 d_secondsUntilPenaltyLifted;
  };

  class TeamInfo
  {
  public:
    uint8 getTeamNumber() const { return d_teamNumber; }
    uint8 isBlueTeam() const { return d_teamColour == 0; }
    uint8 getGoalColour() const { return d_goalColour; }
    uint8 getScore() const { return d_score; }
    PlayerInfo const& getPlayer(uint8 unum) const
    {
      assert(unum > 0 && unum <= MAX_NUM_PLAYERS);
      return d_players[unum - 1];
    }

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
    GameState(char const* data)
    {
      memcpy(&d_data, data, sizeof(GameStateData));
    }

    int getSecondsRemaining() const { return d_data.secondsRemaining; }
    PlayMode getPlayMode() const { return PlayMode(d_data.playMode); }
    std::string getPlayModeString() const
    {
      switch (PlayMode(d_data.playMode))
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

    uint32 getVersion() const { return d_data.version; }
    uint8 getPlayersPerTeam() const { return d_data.playersPerTeam; }
    bool isFirstHalf() const { return d_data.isFirstHalf == 1; }
    /** The next team to kick off. */
    uint8 getNextKickOffTeamNumber() const { return d_data.nextKickOffTeamNumber; }
    bool isPenaltyShootout() const { return ExtraState(d_data.secondaryState) == ExtraState::PENALTYSHOOT; }
    bool isOvertime() const { return ExtraState(d_data.secondaryState) == ExtraState::OVERTIME; }
    uint8 getSecondsSinceLastDropIn() const { return d_data.secondsSinceLastDropIn; }
    uint8 getLastDropInTeamNumber() const { return d_data.dropInTeamNumber; }
    TeamInfo const& teamInfo1() const { return d_data.teams[0]; }
    TeamInfo const& teamInfo2() const { return d_data.teams[1]; }

    TeamInfo const& teamInfo(unsigned teamNumber) const
    {
      return teamInfo1().getTeamNumber() == teamNumber ? teamInfo1() : teamInfo2();
    }

    PlayerInfo const& playerInfo(unsigned team, unsigned unum) const
    {
      assert(team < 2);
      return d_data.teams[team].getPlayer(unum);
    }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    struct GameStateData
    {
      // FIELDS DESERIALISED FROM MEMORY -- DO NOT CHANGE
      char header[4];          // header to identify the structure
      uint32 version;          // version of the data structure
      uint8 playersPerTeam;    // The number of players on a team
      uint8 playMode;       // state of the game (STATE_READY, STATE_PLAYING, etc)
      uint8 isFirstHalf;       // 1 = game in first half, 0 otherwise
      uint8 nextKickOffTeamNumber;       // the next team to kick off
      uint8 secondaryState; // Extra state information - (STATE2_NORMAL, STATE2_PENALTYSHOOT, etc)
      uint8 dropInTeamNumber;        // team that caused last drop in
      uint16 secondsSinceLastDropIn;       // number of seconds passed since the last drop in.  -1 before first dropin
      uint32 secondsRemaining; // estimate of number of seconds remaining in the half
      TeamInfo teams[2];
    };

    GameStateData d_data;
  };
}
