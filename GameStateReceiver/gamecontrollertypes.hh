#pragma once

#include <string>
#include <string.h>

#include "../util/assert.hh"

typedef unsigned char  uint8;
typedef short          int16;
typedef unsigned short uint16;
typedef unsigned int   uint32;

namespace robocup
{
  enum class PlayMode : uint8
  {
    INITIAL = 0,
    READY = 1,
    SET = 2,
    PLAYING = 3,
    FINISHED = 4
  };

  inline std::string getPlayModeName(PlayMode playMode)
  {
    switch (playMode)
    {
      case PlayMode::INITIAL:  return "Initial";
      case PlayMode::READY:    return "Ready";
      case PlayMode::SET:      return "Set";
      case PlayMode::PLAYING:  return "Playing";
      case PlayMode::FINISHED: return "Finished";
      default:
        throw std::runtime_error("Unsupported PlayMode enum value.");
    }
  }

  enum class PeriodType : uint8
  {
    NORMAL = 0,
    PENALTY_SHOOTOUT = 1,
    OVERTIME = 2,
    TIMEOUT = 3
  };

  inline std::string getPeriodTypeName(PeriodType periodType)
  {
    switch (periodType)
    {
      case PeriodType::NORMAL:           return "Normal";
      case PeriodType::PENALTY_SHOOTOUT: return "Penalty shootout";
      case PeriodType::OVERTIME:         return "Overtime";
      case PeriodType::TIMEOUT:          return "Timeout";
      default:
        throw std::runtime_error("Unsupported PeriodType enum value.");
    }
  }

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

  inline std::string getPenaltyTypeName(PenaltyType penaltyType)
  {
    switch (penaltyType)
    {
      case PenaltyType::NONE:                return "No Penalty";
      case PenaltyType::BALL_MANIPULATION:   return "Ball Manipulation";
      case PenaltyType::PHYSICAL_CONTACT:    return "Physical Contact";
      case PenaltyType::ILLEGAL_ATTACK:      return "Illegal Attack";
      case PenaltyType::ILLEGAL_DEFENSE:     return "Illegal Defense";
      case PenaltyType::PICKUP_OR_INCAPABLE: return "Pickup or Incapable";
      case PenaltyType::SERVICE:             return "Service";
      case PenaltyType::SUBSTITUTE:          return "Substitute";
      case PenaltyType::MANUAL:              return "Manual";
      default:
        throw std::runtime_error("Unsupported PenaltyType enum value.");
    }
  }

  enum class League : uint8
  {
    SPL = 0x01,
    SPLDropIn = 0x02,
    HumanoidKidSize = 0x11,
    HumanoidTeenSize = 0x12,
    HumanoidAdultSize = 0x13
  };

  inline std::string getLeagueName(League league)
  {
    switch (league)
    {
      case League::SPL:               return "SPL";
      case League::SPLDropIn:         return "SPL Drop In";
      case League::HumanoidKidSize:   return "Humanoid Kid Size";
      case League::HumanoidTeenSize:  return "Humanoid Teen Size";
      case League::HumanoidAdultSize: return "Humanoid Adult Size";
      default:
        throw std::runtime_error("Unsupported League enum value.");
    }
  }

  /**
  * Structure of player information in the game controller's GameState message.
  */
  struct PlayerInfo
  {
    bool hasPenalty() const { return d_penaltyType != PenaltyType::NONE; }
    PenaltyType getPenaltyType() const { return d_penaltyType; }
    /** estimate of time till unpenalised */
    uint8 getSecondsUntilPenaltyLifted() const { return d_secondsUntilPenaltyLifted; }

    static constexpr uint8 SIZE = 2;

  private:
    // FIELDS DESERIALISED FROM MEMORY -- DO NOT CHANGE
    PenaltyType d_penaltyType;
    uint8 d_secondsUntilPenaltyLifted;
  };

  /**
   * Structure of team information in the game controller's GameState message.
   */
  struct TeamInfo
  {
    uint8 getTeamNumber() const { return d_teamNumber; }
    uint8 isBlueTeam() const { return d_teamColour == 0; }
    uint8 getScore() const { return d_score; }
    uint8 getPenaltyShotCount() const { return d_penaltyShot; }

    bool wasPenaltySuccessful(uint8 number) const
    {
      ASSERT(number < d_penaltyShot);
      return ((1 << number) & d_singleShots) != 0;
    }

    PlayerInfo const& getPlayer(uint8 unum) const
    {
      ASSERT(unum > 0 && unum <= PLAYER_COUNT);
      return d_players[unum - 1];
    }

    static constexpr uint8 PLAYER_COUNT = 11;
    static constexpr uint8 SIZE = 6 + PLAYER_COUNT*PlayerInfo::SIZE;

  private:
    // FIELDS DESERIALISED FROM MEMORY -- DO NOT CHANGE
    uint8 d_teamNumber;   // Unique team number
    uint8 d_teamColour;   // Colour of the team
    uint8 d_score;        // Team's score
    uint8 d_penaltyShot;  // Penalty shot counter
    uint16 d_singleShots; // Bits represent penalty shot success
    PlayerInfo d_players[PLAYER_COUNT]; // The team's players
  };

  /**
   * Message sent from game controller to robots (version 9).
   */
  struct GameStateMessage
  {
    // FIELDS DESERIALISED FROM MEMORY -- DO NOT CHANGE
    uint32 header;                 // Header to identify the structure
    uint8 version;                 // Version of the data structure
    League leagueNumber;           // Identifies the league of the current game
    uint8 packetNumber;            // Sequence number of the packet (overflows from 255 to 0)
    uint32 gameControllerId;       // A 32-bit number that identifies this game controller (use to detect when multiple GCs are running)
    uint8 playersPerTeam;          // The number of players on a team
    PlayMode playMode;             // The game's play mode (initial, ready, set, play, finished)
    uint8 isFirstHalf;             // Whether the first half (1) or second half (0), for both normal and extra game periods
    uint8 nextKickOffTeamIndex;    // Index of the next team to kick off (0 or 1)
    PeriodType periodType;         // The type of game period (normal, extra, penalties, timeout)
    uint8 dropInTeamIndex;         // Index of the team that caused the last drop in (or 2 if no drop in yet)
    int16 secondsSinceLastDropIn;  // Number of seconds passed since the last drop in (or -1 if no drop in yet)
    int16 secondsRemaining;        // Estimate of number of seconds remaining in the half
    int16 secondaryTime;           // Sub-time (remaining in ready state, etc.) in seconds
    TeamInfo teams[2];

    static constexpr uint32 HEADER_INT = 0x656d4752; // "RGme"
    static constexpr uint8 VERSION = 9;
    static constexpr uint8 SIZE = 23 + 2*TeamInfo::SIZE;
  };

  enum class RobotStatusMessageType : uint8
  {
    MANUALLY_PENALISED = 0,
    MANUALLY_UNPENALISED = 1,
    ALIVE = 2
  };

  /**
   * Message sent from robots to the game controller (version 2).
   *
   * Also known as RoboCupGameControlReturnData.
   */
  struct RobotStatusMessage
  {
    RobotStatusMessage(uint8 teamNumber, uint8 uniformNumber, RobotStatusMessageType message)
    : header(HEADER_INT),
      version(VERSION),
      teamNumber(teamNumber),
      uniformNumber(uniformNumber),
      message(message)
    {}

    uint32 header;
    uint8 version;
    uint8 teamNumber;
    uint8 uniformNumber;
    RobotStatusMessageType message;

    static constexpr uint32 HEADER_INT = 0x74724752; // "RGrt"
    static constexpr uint8 VERSION = 2;
    static constexpr uint8 SIZE = 8;
  };
}
