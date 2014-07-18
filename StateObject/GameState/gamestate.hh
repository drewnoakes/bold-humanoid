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
    GameState(char const* data)
    : d_receivedAt(Clock::getTimestamp())
    {
      memcpy(&d_data, data, sizeof(robocup::GameStateData));
    }

    robocup::PlayMode getPlayMode() const { return robocup::PlayMode(d_data.playMode); }

    uint8 getVersion() const { return d_data.version; }
    uint8 getPacketNumber() const { return d_data.packetNumber; }
    uint8 getPlayersPerTeam() const { return d_data.playersPerTeam; }
    bool isFirstHalf() const { return d_data.isFirstHalf == 1; }
    /** Index of the next team to kick off. Either zero or one. */
    uint8 getNextKickOffTeamIndex() const { return d_data.nextKickOffTeamIndex; }
    bool isPenaltyShootout() const { return robocup::ExtraState(d_data.secondaryState) == robocup::ExtraState::PENALTYSHOOT; }
    bool isOvertime() const { return robocup::ExtraState(d_data.secondaryState) == robocup::ExtraState::OVERTIME; }
    bool isTimeout() const { return robocup::ExtraState(d_data.secondaryState) == robocup::ExtraState::TIMEOUT; }
    bool isClockRunning() const { return getPlayMode() == robocup::PlayMode::PLAYING; }
    uint8 getLastDropInTeamNumber() const { return d_data.dropInTeamNumber; }
    int16 getSecondsSinceLastDropIn() const { return d_data.secondsSinceLastDropIn + (isClockRunning() ? Clock::getSecondsSince(d_receivedAt) : 0); }
    int16 getSecondsRemaining() const { return d_data.secondsRemaining - (isClockRunning() ? Clock::getSecondsSince(d_receivedAt) : 0); }
    int16 getSecondaryTime() const
    {
      int16 secondaryTime = d_data.secondaryTime;
      if (isClockRunning())
        secondaryTime -= Clock::getSecondsSince(d_receivedAt);
      return secondaryTime > 0 ? secondaryTime : 0;
    }

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

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
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
    robocup::GameStateData d_data;
  };
}
