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

  enum class ExtraState : uint8
  {
    NORMAL = 0,
    PENALTYSHOOT = 1,
    OVERTIME = 2,
    TIMEOUT = 3
  };

  inline std::string getExtraStateName(ExtraState extraState)
  {
    switch (extraState)
    {
      case ExtraState::NORMAL:       return "Normal";
      case ExtraState::PENALTYSHOOT: return "Penalty shoot";
      case ExtraState::OVERTIME:     return "Overtime";
      case ExtraState::TIMEOUT:      return "Timeout";
      default:
        throw std::runtime_error("Unsupported ExtraState enum value.");
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

  /// Model of the PlayerInfo struct (version 9)
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

  /// Model of the TeamInfo struct (version 9)
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

    static constexpr uint8 SIZE = 6 + MAX_NUM_PLAYERS*PlayerInfo::SIZE;

  private:
    // FIELDS DESERIALISED FROM MEMORY -- DO NOT CHANGE
    uint8 d_teamNumber;   // Unique team number
    uint8 d_teamColour;   // Colour of the team
    uint8 d_score;        // Team's score
    uint8 d_penaltyShot;  // Penalty shot counter
    uint16 d_singleShots; // Bits represent penalty shot success
    PlayerInfo d_players[MAX_NUM_PLAYERS]; // the team's players
  };

  /**
   * Message sent from game controller to robots (version 9).
   */
  struct GameStateMessage
  {
    // FIELDS DESERIALISED FROM MEMORY -- DO NOT CHANGE
    char header[4];                // Header to identify the structure
    uint8 version;                 // Version of the data structure
    uint8 leagueNumber;            // Identifies the league of the current game
    uint8 packetNumber;            // Sequence number of the packet (overflows from 255 to 0)
    uint32 gameControllerId;       // A 32-bit number that identifies this game controller (use to detect when multiple GCs are running)
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
