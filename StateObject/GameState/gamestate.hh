#pragma once

#include "../stateobject.hh"

#include "../../Clock/clock.hh"
#include "../../util/assert.hh"
#include "../../Config/config.hh"

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <string>

namespace bold
{
  enum class PlayMode : uint8_t
  {
    INITIAL = 0,
    READY = 1,
    SET = 2,
    PLAYING = 3,
    FINISHED = 4
  };

  enum class PeriodType : uint8_t
  {
    NORMAL = 0,
    PENALTY_SHOOTOUT = 1,
    OVERTIME = 2,
    TIMEOUT = 3
  };

  enum class PenaltyType : uint8_t
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

  enum class League : uint8_t
  {
    SPL = 0x01,
    SPLDropIn = 0x02,
    HumanoidKidSize = 0x11,
    HumanoidTeenSize = 0x12,
    HumanoidAdultSize = 0x13
  };

  enum class TeamColor : uint8_t
  {
    Blue = 0,
    Red = 1
  };

  enum class Team : uint8_t
  {
    Us,
    Them
  };

  enum class GameResult : uint8_t
  {
    Undecided = 0,
    Victory = 1,
    Loss = 2,
    Draw = 3
  };

  enum class GameControllerRobotStatus : uint8_t
  {
    MANUALLY_PENALISED = 0,
    MANUALLY_UNPENALISED = 1,
    ALIVE = 2
  };

  enum class RobotStatusMessageType : uint8_t
  {
    MANUALLY_PENALISED = 0,
    MANUALLY_UNPENALISED = 1,
    ALIVE = 2
  };

  std::string getPlayModeName(PlayMode playMode);
  std::string getPeriodTypeName(PeriodType periodType);
  std::string getPenaltyTypeName(PenaltyType penaltyType);
  std::string getLeagueName(League league);
  std::string getTeamColorName(TeamColor teamColor);
  std::string getGameResultName(GameResult gameResult);
  std::string getRobotStatusMessageTypeName(RobotStatusMessageType status);

  class PlayerData
  {
  public:
    PlayerData(PenaltyType penaltyType, uint8_t secondsUntilPenaltyLifted)
      :  d_penaltyType(penaltyType),
         d_secondsUntilPenaltyLifted(secondsUntilPenaltyLifted)
    {}

    PlayerData() = default;

    bool hasPenalty() const { return d_penaltyType != PenaltyType::NONE; }

    PenaltyType getPenaltyType() const { return d_penaltyType; }

    /** An estimate of time till unpenalised */
    uint8_t getSecondsUntilPenaltyLifted() const { return d_secondsUntilPenaltyLifted; }

  private:
    PenaltyType d_penaltyType;
    uint8_t d_secondsUntilPenaltyLifted;
  };

  class TeamData
  {
  public:
    TeamData() = default;

    uint8_t getTeamNumber() const { return d_teamNumber; }
    TeamColor getTeamColor() const { return d_teamColour; }
    bool isBlueTeam() const { return d_teamColour == TeamColor::Blue; }
    uint8_t getScore() const { return d_score; }
    uint8_t getPenaltyShotCount() const { return d_penaltyShot; }

    bool wasPenaltySuccessful(uint8_t number) const
    {
      ASSERT(number < d_penaltyShot);
      return ((1 << number) & d_singleShots) != 0;
    }

    PlayerData const& getPlayer(uint8_t unum) const
    {
      ASSERT(unum > 0 && unum <= d_players.size());
      return d_players[unum - 1];
    }

    static constexpr uint8_t PLAYER_COUNT = 11;

  private:
    friend class GameStateDecoderVersion7;
    friend class GameStateDecoderVersion8;

    uint8_t d_teamNumber;   // Unique team number
    TeamColor d_teamColour; // Colour of the team
    uint8_t d_score;        // Team's score
    uint8_t d_penaltyShot;  // Penalty shot counter
    uint16_t d_singleShots; // Bits represent penalty shot success
    std::vector<PlayerData> d_players; // The team's players
  };

  class GameState : public StateObject
  {
  public:
    GameState() = default;

    PlayMode getPlayMode() const { return d_playMode; }
    uint8_t getVersion() const { return d_version; }
//    League getLeague() const { return d_league; }
    uint8_t getPacketNumber() const { return d_packetNumber; }
//    uint32_t getGameControllerId() const { return d_gameControllerId; }
    uint8_t getPlayersPerTeam() const { return d_playersPerTeam; }
    bool isFirstHalf() const { return d_isFirstHalf; }
    /** Index of the next team to kick off. Either zero or one. */
    uint8_t getNextKickOffTeamIndex() const { return d_nextKickOffTeamIndex; }
    bool isPenaltyShootout() const { return d_periodType == PeriodType::PENALTY_SHOOTOUT; }
    bool isOvertime() const { return d_periodType == PeriodType::OVERTIME; }
    bool isTimeout() const { return d_periodType == PeriodType::TIMEOUT; }
    bool isClockRunning() const { return d_playMode == PlayMode::PLAYING; }
    /** Identifies the team having the last drop in (0 blue, 1 red, 2 no drop in yet). */
    uint8_t getLastDropInTeamColorNumber() const { return d_lastDropInTeamColorNumber; }
    int16_t getSecondsSinceLastDropIn() const { return d_secondsSinceLastDropIn + (isClockRunning() ? Clock::getSecondsSince(d_receivedAt) : 0); }
    int16_t getSecondsRemaining() const { return d_secondsRemaining - (isClockRunning() ? Clock::getSecondsSince(d_receivedAt) : 0); }

    int16_t getSecondaryTime() const
    {
      int16_t secondaryTime = d_secondaryTime;
      if (isClockRunning())
        secondaryTime -= Clock::getSecondsSince(d_receivedAt);
      return secondaryTime > 0 ? secondaryTime : (int16_t)0;
    }

    bool isWithinTenSecondsOfKickOff(Team team) const;

    TeamData const& getTeam1() const { return d_team1; }
    TeamData const& getTeam2() const { return d_team2; }

    TeamData const& getMyTeam() const
    {
      static auto teamNumber = (uint8_t)Config::getStaticValue<int>("team-number");
      return getTeam(teamNumber);
    }

    TeamData const& getOpponentTeam() const
    {
      static auto teamNumber = (uchar)Config::getStaticValue<int>("team-number");
      return getTeamIndex(teamNumber) == 0 ? d_team2 : d_team1;
    }

    PlayerData const& getMyPlayerInfo() const
    {
      static int unum = Config::getStaticValue<int>("uniform-number");
      return getMyTeam().getPlayer(unum);
    }

    double getAgeMillis() const { return Clock::getMillisSince(d_receivedAt); }

    GameResult getGameResult() const;

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }

  private:
    friend class GameStateDecoderVersion7;
    friend class GameStateDecoderVersion8;

    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    TeamData const& getTeam(uchar teamNumber) const
    {
      return getTeamIndex(teamNumber) == 0 ? d_team1 : d_team2;
    }

    uchar getTeamIndex(uchar teamNumber) const
    {
      if (d_team1.getTeamNumber() == teamNumber)
        return (uchar)0;
      if (d_team2.getTeamNumber() == teamNumber)
        return (uchar)1;

      // We should never reach this point as the GameStateReceiver should not propagate meesages which do not
      // apply to our team.
      log::error("GameState::getTeamIndex") << "Attempt to get index for unknown team number " << teamNumber;
      throw std::runtime_error("Attempt to get index for unknown team number.");
    }

//  League d_league;                   // Identifies the league of the current game
    uint8_t d_packetNumber;              // Sequence number of the packet (overflows from 255 to 0)
    uint8_t d_playersPerTeam;            // The number of players on a team
    uint8_t d_version;                   // Version of the GameState protocol used
//  uint32 d_gameControllerId;         // A 32-bit number that identifies this game controller (use to detect when multiple GCs are running)
    PlayMode d_playMode;               // The game's play mode (initial, ready, set, play, finished)
    bool d_isFirstHalf;                // Whether the first half (1) or second half (0), for both normal and extra game periods
    uint8_t d_nextKickOffTeamIndex;      // Index of the next team to kick off (0 or 1)
    PeriodType d_periodType;           // The type of game period (normal, extra, penalties, timeout)
    uint8_t d_lastDropInTeamColorNumber; // Color of the team that caused the last drop in (or 2 if no drop in yet)
//  uint8_t d_isKnockOutGame;            // Whether the game is a knockout (1) or not (0)
    int16_t d_secondsSinceLastDropIn;    // Number of seconds passed since the last drop in (or -1 if no drop in yet)
    int16_t d_secondsRemaining;          // Estimate of number of seconds remaining in the half
    int16_t d_secondaryTime;             // Sub-time (remaining in ready state, etc.) in seconds

    TeamData d_team1;
    TeamData d_team2;

    Clock::Timestamp d_receivedAt;
  };

  template<typename TBuffer>
  inline void GameState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
  {
    writer.StartObject();
    {
      writer.String("playMode");
      writer.String(getPlayModeName(getPlayMode()).c_str());
      writer.String("packet");
      writer.Uint(getPacketNumber());
//      writer.String("gameControllerId");
//      writer.Int(getGameControllerId());
      writer.String("playerPerTeam");
      writer.Uint(getPlayersPerTeam());
      writer.String("isFirstHalf");
      writer.Bool(isFirstHalf());
      writer.String("nextKickOffTeamIndex");
      writer.Uint(getNextKickOffTeamIndex());
      writer.String("isPenaltyShootOut");
      writer.Bool(isPenaltyShootout());
      writer.String("isOvertime");
      writer.Bool(isOvertime());
      writer.String("isTimeout");
      writer.Bool(isTimeout());
      writer.String("lastDropInTeamColor");
      writer.Uint(getLastDropInTeamColorNumber());
      writer.String("secSinceDropIn");
      writer.Int(getSecondsSinceLastDropIn());
      writer.String("secondsRemaining");
      writer.Int(getSecondsRemaining());
      writer.String("secondsSecondaryTime");
      writer.Int(getSecondaryTime());

      auto writeTeam = [&writer,this](TeamData const& team)
      {
        writer.StartObject();
        {
          writer.String("num");
          writer.Uint(team.getTeamNumber());
          writer.String("isBlue");
          writer.Bool(team.isBlueTeam());
          writer.String("score");
          writer.Uint(team.getScore());
          writer.String("penaltyShotCount");
          writer.Uint(team.getPenaltyShotCount());

          writer.String("players");
          writer.StartArray();
          {
            for (uint8_t p = 1; p <= getPlayersPerTeam(); ++p)
            {
              auto const& player = team.getPlayer(p);
              writer.StartObject();
              {
                writer.String("penalty");
                if (player.getPenaltyType() == PenaltyType::NONE) {
                  writer.Null();
                } else {
                  writer.String(getPenaltyTypeName(player.getPenaltyType()).c_str());

                  writer.String("penaltySecondsRemaining");
                  writer.Uint(player.getSecondsUntilPenaltyLifted());
                }
              }
              writer.EndObject();
            }
          }
          writer.EndArray();
        }
        writer.EndObject();
      };

      writer.String("myTeam");
      writeTeam(getMyTeam());

      writer.String("opponentTeam");
      writeTeam(getOpponentTeam());
    }
    writer.EndObject();
  }
}
