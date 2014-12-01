#pragma once

#include "../stateobject.hh"

#include "../../Clock/clock.hh"
#include "../../GameStateReceiver/gamecontrollertypes.hh"
#include "../../util/assert.hh"
#include "../../Config/config.hh"

#include <stdexcept>
#include <vector>
#include <string>

namespace bold
{
  enum class Team
  {
    Us,
    Them
  };

  enum class GameResult
  {
    Undecided = 0,
    Victory = 1,
    Loss = 2,
    Draw = 3
  };

  inline std::string getGameResultName(GameResult gameResult)
  {
    switch (gameResult)
    {
      case GameResult::Undecided: return "Undecided";
      case GameResult::Victory:   return "Victory";
      case GameResult::Loss:      return "Loss";
      case GameResult::Draw:      return "Draw";
    }
    return "Unknown";
  }

  class GameState : public StateObject
  {
  public:
    GameState(char const* data);

    robocup::PlayMode getPlayMode() const { return d_data.playMode; }

    uint8 getVersion() const { return d_data.version; }
    robocup::League getLeague() const { return d_data.league; }
    uint8 getPacketNumber() const { return d_data.packetNumber; }
    uint8 getPlayersPerTeam() const { return d_data.playersPerTeam; }
    bool isFirstHalf() const { return d_data.isFirstHalf == 1; }
    /** Index of the next team to kick off. Either zero or one. */
    uint8 getNextKickOffTeamIndex() const { return d_data.nextKickOffTeamIndex; }
    bool isPenaltyShootout() const { return d_data.periodType == robocup::PeriodType::PENALTY_SHOOTOUT; }
    bool isOvertime() const { return d_data.periodType == robocup::PeriodType::OVERTIME; }
    bool isTimeout() const { return d_data.periodType == robocup::PeriodType::TIMEOUT; }
    bool isClockRunning() const { return getPlayMode() == robocup::PlayMode::PLAYING; }
    /** Identifies the team having the last drop in (0 blue, 1 red, 2 no drop in yet). */
    uint8 getLastDropInTeamColorNumber() const { return d_data.dropInTeamColor; }
    int16 getSecondsSinceLastDropIn() const { return d_data.secondsSinceLastDropIn + (isClockRunning() ? Clock::getSecondsSince(d_receivedAt) : 0); }
    int16 getSecondsRemaining() const { return d_data.secondsRemaining - (isClockRunning() ? Clock::getSecondsSince(d_receivedAt) : 0); }

    int16 getSecondaryTime() const
    {
      int16 secondaryTime = d_data.secondaryTime;
      if (isClockRunning())
        secondaryTime -= Clock::getSecondsSince(d_receivedAt);
      return secondaryTime > 0 ? secondaryTime : 0;
    }
    uint32 getGameControllerId() const { return d_data.gameControllerId; }

    bool isWithinTenSecondsOfKickOff(Team team) const;

    robocup::TeamInfo const& getTeam1() const { return d_data.teams[0]; }
    robocup::TeamInfo const& getTeam2() const { return d_data.teams[1]; }

    robocup::TeamInfo const& getMyTeam() const
    {
      static auto teamNumber = Config::getStaticValue<int>("team-number");
      return getTeam(teamNumber);
    }

    robocup::TeamInfo const& getOpponentTeam() const
    {
      static auto teamNumber = Config::getStaticValue<int>("team-number");
      return d_data.teams[getTeamIndex(teamNumber) ^ 1];
    }

    robocup::PlayerInfo const& getMyPlayerInfo() const
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
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    robocup::TeamInfo const& getTeam(uchar teamNumber) const
    {
      return d_data.teams[getTeamIndex(teamNumber)];
    }

    uchar getTeamIndex(uchar teamNumber) const
    {
      if (d_data.teams[0].getTeamNumber() == teamNumber)
        return (uchar)0;
      if (d_data.teams[1].getTeamNumber() == teamNumber)
        return (uchar)1;

      // We should never reach this point as the GameStateReceiver should not propagate meesages which do not
      // apply to our team.
      log::error("GameState::getTeamIndex") << "Attempt to get index for unknown team number " << teamNumber;
      throw std::runtime_error("Attempt to get index for unknown team number.");
    }

    Clock::Timestamp d_receivedAt;
    robocup::GameStateMessage d_data;
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
      writer.String("gameControllerId");
      writer.Int(getGameControllerId());

      auto writeTeam = [&writer,this](robocup::TeamInfo const& team)
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
            for (int p = 1; p <= getPlayersPerTeam(); ++p) {
              auto const& player = team.getPlayer(p);
              writer.StartObject();
              {
                writer.String("penalty");
                if (player.getPenaltyType() == robocup::PenaltyType::NONE) {
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
