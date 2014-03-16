#pragma once

#include "../stateobject.hh"

#include "../../GameStateReceiver/gamecontrollertypes.hh"

#include <cassert>
#include <stdexcept>
#include <vector>
#include <string>

namespace bold
{
  class GameState : public StateObject
  {
  public:
    GameState(char const* data)
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
    int16 getSecondsSinceLastDropIn() const { return d_data.secondsSinceLastDropIn; }
    int16 getSecondsRemaining() const { return d_data.secondsRemaining; }
    int16 getSecondaryTime() const { return d_data.secondaryTime; }

    robocup::TeamInfo const& teamInfo1() const { return d_data.teams[0]; }
    robocup::TeamInfo const& teamInfo2() const { return d_data.teams[1]; }

    robocup::TeamInfo const& teamInfo(unsigned teamNumber) const
    {
      return teamInfo1().getTeamNumber() == teamNumber ? teamInfo1() : teamInfo2();
    }

    robocup::PlayerInfo const& playerInfo(unsigned team, unsigned unum) const
    {
      assert(team < 2);
      return d_data.teams[team].getPlayer(unum);
    }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    robocup::GameStateData d_data;
  };
}
