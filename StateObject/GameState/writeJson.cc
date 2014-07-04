#include "gamestate.hh"

using namespace bold;
using namespace rapidjson;
using namespace robocup;
using namespace std;

bool GameState::isWithinTenSecondsOfKickOff(Team team) const
{
  static uchar teamNumber = static_cast<uchar>(Config::getStaticValue<int>("team-number"));
  uchar nextKickOffTeamIndex = getNextKickOffTeamIndex();

  bool isOurKickOff = nextKickOffTeamIndex == getTeamIndex(teamNumber);
  bool isOurTeam = team == Team::Us;

  PlayMode playMode = getPlayMode();
  int secondaryTime = getSecondaryTime();

  return isOurTeam == isOurKickOff && playMode == PlayMode::PLAYING && secondaryTime > 0;
}

void GameState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("playMode").String(getPlayModeString().c_str());
    writer.String("packet").Uint(getPacketNumber());
    writer.String("playerPerTeam").Int(getPlayersPerTeam());
    writer.String("isFirstHalf").Bool(isFirstHalf());
    writer.String("nextKickOffTeamIndex").Int(getNextKickOffTeamIndex());
    writer.String("isPenaltyShootOut").Bool(isPenaltyShootout());
    writer.String("isOvertime").Bool(isOvertime());
    writer.String("isTimeout").Bool(isTimeout());
    writer.String("lastDropInTeamNum").Int(getLastDropInTeamNumber());
    writer.String("secSinceDropIn").Int(getSecondsSinceLastDropIn());
    writer.String("secondsRemaining").Int(getSecondsRemaining());
    writer.String("secondsSecondaryTime").Int(getSecondaryTime());

    auto writeTeam = [&writer,this](TeamInfo const& team)
    {
      writer.StartObject();
      {
        writer.String("num").Int(team.getTeamNumber());
        writer.String("isBlue").Bool(team.isBlueTeam());
        writer.String("score").Int(team.getScore());
        writer.String("penaltyShotCount").Int(team.getPenaltyShotCount());

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
    writeTeam(getTeam1());

    writer.String("team2");
    writeTeam(getTeam2());
  }
  writer.EndObject();
}
