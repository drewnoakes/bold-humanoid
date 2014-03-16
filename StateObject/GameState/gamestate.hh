#pragma once

#include "../stateobject.hh"

#include <cassert>
#include <stdexcept>
#include <vector>
#include <string>

namespace bold
{
  #define MAX_NUM_PLAYERS 11

  typedef unsigned char  uint8;
  typedef short          int16;
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
    OVERTIME = 2,
    TIMEOUT = 3
  };

  enum class PenaltyType : uint8
  {
    NONE = 0,
    BALL_MANIPULATION = 1,
    PHYSICAL_CONTACT = 2,
    ILLEGAL_ATTACK = 3,
    ILLEGAL_DEFENSE = 4,
    PICKUP_OR_INCAPABLE = 5,
    SERVICE = 6,
    SUBSTITUTE = 14,
    MANUAL = 15
  };

  /// Model of the PlayerInfo struct (version 8)
  struct PlayerInfo
  {
    bool hasPenalty() const { return d_penaltyType != PenaltyType::NONE; }
    PenaltyType getPenaltyType() const { return d_penaltyType; }
    std::string getPenaltyTypeString() const
    {
      switch (d_penaltyType)
      {
        case PenaltyType::NONE:
          return std::string("No Penalty");
        case PenaltyType::BALL_MANIPULATION:
          return std::string("Ball Manipulation");
        case PenaltyType::PHYSICAL_CONTACT:
          return std::string("Physical Contact");
        case PenaltyType::ILLEGAL_ATTACK:
          return std::string("Illegal Attack");
        case PenaltyType::ILLEGAL_DEFENSE:
          return std::string("Illegal Defense");
        case PenaltyType::PICKUP_OR_INCAPABLE:
          return std::string("Pickup or Incapable");
        case PenaltyType::SERVICE:
          return std::string("Service");
        case PenaltyType::SUBSTITUTE:
          return std::string("Substitute");
        case PenaltyType::MANUAL:
          return std::string("Manual");
        default:
          throw new std::runtime_error("Unsupported PenaltyType enum value.");
      }
    }
    /** estimate of time till unpenalised */
    uint8 getSecondsUntilPenaltyLifted() const { return d_secondsUntilPenaltyLifted; }

    static constexpr uint8 VERSION = 8;
    static constexpr uint8 SIZE = 2;

  private:
    // FIELDS DESERIALISED FROM MEMORY -- DO NOT CHANGE
    PenaltyType d_penaltyType;
    uint8 d_secondsUntilPenaltyLifted;
  };

  /// Model of the TeamInfo struct (version 8)
  struct TeamInfo
  {
    uint8 getTeamNumber() const { return d_teamNumber; }
    uint8 isBlueTeam() const { return d_teamColour == 0; }
    uint8 getScore() const { return d_score; }
    uint8 getPenaltyShotCount() const { return d_penaltyShot; }
    bool wasPenaltySuccessful(uint8 number) const
    {
      assert(number < d_penaltyShot);
      return ((1 << number) & d_singleShots) != 0;
    }

    PlayerInfo const& getPlayer(uint8 unum) const
    {
      assert(unum > 0 && unum <= MAX_NUM_PLAYERS);
      return d_players[unum - 1];
    }

    static constexpr uint8 VERSION = 8;
    static constexpr uint8 SIZE = 46 + (1+MAX_NUM_PLAYERS)*PlayerInfo::SIZE;

  private:
    // FIELDS DESERIALISED FROM MEMORY -- DO NOT CHANGE
    uint8 d_teamNumber;   // Unique team number
    uint8 d_teamColour;   // Colour of the team
    uint8 d_score;        // Team's score
    uint8 d_penaltyShot;  // Penalty shot counter
    uint16 d_singleShots; // Bits represent penalty shot success
    uint8 d_coachMessage[40]; // For SPL only (ignore in kid-size league)
    PlayerInfo d_coach;       // For SPL only (ignore in kid-size league)
    PlayerInfo d_players[MAX_NUM_PLAYERS]; // the team's players
  };

  /// Model of the GameStateData struct (version 8)
  struct GameStateData
  {
    // FIELDS DESERIALISED FROM MEMORY -- DO NOT CHANGE
    char header[4];                // Header to identify the structure
    uint8 version;                 // Version of the data structure
    uint8 packetNumber;            //
    uint8 playersPerTeam;          // The number of players on a team
    uint8 playMode;                // state of the game (STATE_READY, STATE_PLAYING, etc)
    uint8 isFirstHalf;             // 1 = game in first half, 0 otherwise
    uint8 nextKickOffTeamNumber;   // The next team to kick off
    uint8 secondaryState;          // Extra state information - (STATE2_NORMAL, STATE2_PENALTYSHOOT, etc)
    uint8 dropInTeamNumber;        // Team that caused last drop in
    int16 secondsSinceLastDropIn;  // Number of seconds passed since the last drop in.  -1 before first drop in.
    int16 secondsRemaining;        // Estimate of number of seconds remaining in the half
    int16 secondaryTime;           // Sub-time (remaining in ready state, etc.) in seconds
    TeamInfo teams[2];

    static constexpr const char* HEADER = "RGme";
    static constexpr uint32 HEADER_INT = 0x656d4752;
    static constexpr uint8 VERSION = 8;
    static constexpr uint8 SIZE = 18 + 2*TeamInfo::SIZE;
  };

  class GameState : public StateObject
  {
  public:
    GameState(char const* data)
    {
      memcpy(&d_data, data, sizeof(GameStateData));
    }

    PlayMode getPlayMode() const { return PlayMode(d_data.playMode); }

    std::string getPlayModeString() const
    {
      switch (getPlayMode())
      {
        case PlayMode::INITIAL:
          return std::string("Initial");
        case PlayMode::READY:
          return std::string("Ready");
        case PlayMode::SET:
          return std::string("Set");
        case PlayMode::PLAYING:
          return std::string("Playing");
        case PlayMode::FINISHED:
          return std::string("Finished");
        default:
          throw new std::runtime_error("Unsupported PlayMode enum value.");
      }
    }

    uint8 getVersion() const { return d_data.version; }
    uint8 getPacketNumber() const { return d_data.packetNumber; }
    uint8 getPlayersPerTeam() const { return d_data.playersPerTeam; }
    bool isFirstHalf() const { return d_data.isFirstHalf == 1; }
    /** The next team to kick off. */
    uint8 getNextKickOffTeamNumber() const { return d_data.nextKickOffTeamNumber; }
    bool isPenaltyShootout() const { return ExtraState(d_data.secondaryState) == ExtraState::PENALTYSHOOT; }
    bool isOvertime() const { return ExtraState(d_data.secondaryState) == ExtraState::OVERTIME; }
    bool isTimeout() const { return ExtraState(d_data.secondaryState) == ExtraState::TIMEOUT; }
    uint8 getLastDropInTeamNumber() const { return d_data.dropInTeamNumber; }
    int16 getSecondsSinceLastDropIn() const { return d_data.secondsSinceLastDropIn; }
    int16 getSecondsRemaining() const { return d_data.secondsRemaining; }
    int16 getSecondaryTime() const { return d_data.secondaryTime; }

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
    GameStateData d_data;
  };
}
