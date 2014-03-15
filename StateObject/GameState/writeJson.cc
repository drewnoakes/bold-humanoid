#include "gamestate.hh"

#include "../../GameStateReceiver/RoboCupGameControlData.h"

using namespace bold;
using namespace rapidjson;

static_assert(GameState::InfoSizeBytes == sizeof(RoboCupGameControlData), "");
static_assert(GameState::PongSizeBytes == sizeof(RoboCupGameControlReturnData), "");

void GameState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("playMode");
    writer.String(getPlayModeString().c_str());

    writer.String("playerPerTeam");
    writer.Int(getPlayersPerTeam());

    writer.String("isFirstHalf");
    writer.Bool(isFirstHalf());

    writer.String("nextKickOffTeamNum");
    writer.Int(getNextKickOffTeamNumber());

    writer.String("isPenaltyShootOut");
    writer.Bool(isPenaltyShootout());

    writer.String("isOvertime");
    writer.Bool(isOvertime());

    writer.String("lastDropInTeamNum");
    writer.Int(getLastDropInTeamNumber());

    writer.String("secSinceDropIn");
    writer.Int(getSecondsSinceLastDropIn());

    writer.String("secondsRemaining");
    writer.Int(getSecondsRemaining());

    auto writeTeam = [&writer,this](TeamInfo const& team)
    {
      writer.String("num");
      writer.Int(team.getTeamNumber());

      writer.String("score");
      writer.Int(team.getScore());

      writer.String("players");
      writer.StartArray();
      {
        for (int p = 1; p <= getPlayersPerTeam(); ++p) {
          auto const& player = team.getPlayer(p);
          writer.StartObject();
          {
            writer.String("penalty");
            if (player.getPenaltyType() == PenaltyType::NONE) {
              writer.Null();
            } else {
              writer.String(player.getPenaltyTypeString().c_str());

              writer.String("penaltySecondsRemaining");
              writer.Int(player.getSecondsUntilPenaltyLifted());
            }
          }
          writer.EndObject();
        }
      }
      writer.EndArray();
    };

    writer.String("team1");
    writer.StartObject();
    {
      writeTeam(teamInfo1());
    }
    writer.EndObject();

    writer.String("team2");
    writer.StartObject();
    {
      writeTeam(teamInfo2());
    }
    writer.EndObject();

  }
  writer.EndObject();
}
