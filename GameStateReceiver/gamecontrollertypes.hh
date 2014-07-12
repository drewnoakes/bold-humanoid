#pragma once

#include <string>
#include <string.h>

#include "../util/assert.hh"

#define MAX_NUM_PLAYERS 11

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

  inline std::string getPlayModeString(PlayMode playMode)
  {
    switch (playMode)
    {
      case PlayMode::INITIAL:  return "Initial";
      case PlayMode::READY:    return "Ready";
      case PlayMode::SET:      return "Set";
      case PlayMode::PLAYING:  return "Playing";
      case PlayMode::FINISHED: return "Finished";
      default:
        throw new std::runtime_error("Unsupported PlayMode enum value.");
    }
  }

  enum class ExtraState : uint8
  {
    NORMAL = 0,
    PENALTYSHOOT = 1,
    OVERTIME = 2,
    TIMEOUT = 3
  };

  inline std::string getExtraStateString(ExtraState extraState)
  {
    switch (extraState)
    {
      case ExtraState::NORMAL:       return "Normal";
      case ExtraState::PENALTYSHOOT: return "Penalty shoot";
      case ExtraState::OVERTIME:     return "Overtime";
      case ExtraState::TIMEOUT:      return "Timeout";
      default:
        throw new std::runtime_error("Unsupported ExtraState enum value.");
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

  inline std::string getPenaltyTypeString(PenaltyType penaltyType)
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
        throw new std::runtime_error("Unsupported PenaltyType enum value.");
    }
  }

  /// Model of the PlayerInfo struct (version 8)
  struct PlayerInfo
  {
    bool hasPenalty() const { return d_penaltyType != PenaltyType::NONE; }
    PenaltyType getPenaltyType() const { return d_penaltyType; }
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
      ASSERT(number < d_penaltyShot);
      return ((1 << number) & d_singleShots) != 0;
    }

    PlayerInfo const& getPlayer(uint8 unum) const
    {
      ASSERT(unum > 0 && unum <= MAX_NUM_PLAYERS);
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
    uint8 nextKickOffTeamIndex;    // Index of the next team to kick off (0 or 1)
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

  enum class GameControllerResponseMessage : uint8
  {
    MANUAL_PENALISE = 0,
    MANUAL_UNPENALISE = 1,
    ALIVE = 2
  };

  /// Model of the RoboCupGameControlReturnData struct (version 2)
  struct RoboCupGameControlReturnData
  {
    RoboCupGameControlReturnData(uint8 teamNumber, uint8 uniformNumber, GameControllerResponseMessage message)
    {
      memcpy(&header, HEADER, sizeof(header));
      version = VERSION;

      this->teamNumber = teamNumber;
      this->uniformNumber = uniformNumber;
      this->message = static_cast<uint8>(message);
    }

    char header[4];
    uint8 version;
    uint8 teamNumber;
    uint8 uniformNumber;
    uint8 message;

    static constexpr char const* HEADER = "RGrt";
    static constexpr uint32 HEADER_INT = 0x74724752;
    static constexpr uint8 VERSION = 2;
    static constexpr uint8 SIZE = 8;
  };
}
