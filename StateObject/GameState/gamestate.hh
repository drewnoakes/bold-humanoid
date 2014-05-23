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
  class GameState : public StateObject
  {
  public:
    GameState(char const* data)
    : d_receivedAt(Clock::getTimestamp())
    {
      memcpy(&d_data, data, sizeof(robocup::GameStateData));
    }

    robocup::PlayMode getPlayMode() const { return robocup::PlayMode(d_data.playMode); }

    std::string getPlayModeString() const
    {
      switch (getPlayMode())
      {
        case robocup::PlayMode::INITIAL:
          return std::string("Initial");
        case robocup::PlayMode::READY:
          return std::string("Ready");
        case robocup::PlayMode::SET:
          return std::string("Set");
        case robocup::PlayMode::PLAYING:
          return std::string("Playing");
        case robocup::PlayMode::FINISHED:
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
    bool isPenaltyShootout() const { return robocup::ExtraState(d_data.secondaryState) == robocup::ExtraState::PENALTYSHOOT; }
    bool isOvertime() const { return robocup::ExtraState(d_data.secondaryState) == robocup::ExtraState::OVERTIME; }
    bool isTimeout() const { return robocup::ExtraState(d_data.secondaryState) == robocup::ExtraState::TIMEOUT; }
    uint8 getLastDropInTeamNumber() const { return d_data.dropInTeamNumber; }
    int16 getSecondsSinceLastDropIn() const { return d_data.secondsSinceLastDropIn + (isClockRunning() ? Clock::getSecondsSince(d_receivedAt) : 0); }
    int16 getSecondsRemaining() const { return d_data.secondsRemaining - (isClockRunning() ? Clock::getSecondsSince(d_receivedAt) : 0); }
    int16 getSecondaryTime() const { return d_data.secondaryTime; }

    bool isClockRunning() const
    {
      return getPlayMode() == robocup::PlayMode::PLAYING;
    }

    robocup::TeamInfo const& teamInfo1() const { return d_data.teams[0]; }
    robocup::TeamInfo const& teamInfo2() const { return d_data.teams[1]; }

    unsigned getTeamIndex(unsigned teamNumber) const
    {
      return teamInfo1().getTeamNumber() == teamNumber ? 0 : 1;
    }

    robocup::TeamInfo const& teamInfo(unsigned teamNumber) const
    {
      return d_data.teams[getTeamIndex(teamNumber)];
    }

    robocup::TeamInfo const& ourTeamInfo() const
    {
      static auto teamNumber = Config::getStaticValue<int>("team-number");
      return teamInfo(teamNumber);
    }

    robocup::PlayerInfo const& playerInfo(unsigned teamIndex, unsigned unum) const
    {
      ASSERT(teamIndex < 2);
      return d_data.teams[teamIndex].getPlayer(unum);
    }

    double getAgeMillis() const { return Clock::getMillisSince(d_receivedAt); }

    robocup::PlayerInfo const& myPlayerInfo() const
    {
      static auto teamNumber = Config::getStaticValue<int>("team-number");
      static auto unum = Config::getStaticValue<int>("uniform-number");

      return playerInfo(getTeamIndex(teamNumber), unum);
    }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    Clock::Timestamp d_receivedAt;
    robocup::GameStateData d_data;
  };
}
