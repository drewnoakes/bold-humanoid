#include "gamestate.hh"

using namespace bold;
using namespace rapidjson;

void GameState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("playMode").String(getPlayModeString().c_str());
    writer.String("playerPerTeam").Int(getPlayersPerTeam());
    writer.String("isFirstHalf").Bool(isFirstHalf());
    writer.String("nextKickOffTeamNum").Int(getNextKickOffTeamNumber());
    writer.String("isPenaltyShootOut").Bool(isPenaltyShootout());
    writer.String("isOvertime").Bool(isOvertime());
    writer.String("lastDropInTeamNum").Int(getLastDropInTeamNumber());
    writer.String("secSinceDropIn").Int(getSecondsSinceLastDropIn());
    writer.String("secondsRemaining").Int(getSecondsRemaining());

    auto writeTeam = [&writer,this](TeamInfo const& team)
    {
      writer.StartObject();
      {
        writer.String("num").Int(team.getTeamNumber());
        writer.String("score").Int(team.getScore());

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
      }
      writer.EndObject();
    };

    writer.String("team1");
    writeTeam(teamInfo1());

    writer.String("team2");
    writeTeam(teamInfo2());
  }
  writer.EndObject();
}
