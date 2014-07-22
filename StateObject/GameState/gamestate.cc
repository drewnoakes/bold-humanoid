#include "gamestate.hh"

using namespace bold;
using namespace rapidjson;
using namespace robocup;
using namespace std;

GameResult GameState::getGameResult() const
{
  if (isFirstHalf() || getPlayMode() != PlayMode::FINISHED)
    return GameResult::Undecided;

  uint8 ourScore = getMyTeam().getScore();
  uint8 theirScore = getOpponentTeam().getScore();

  if (ourScore > theirScore)
    return GameResult::Victory;

  if (ourScore < theirScore)
    return GameResult::Loss;

  // TODO had to know if there's going to be extra time or a penalty shoot out in case of draw

  return GameResult::Undecided;
}

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
    writer.String("playMode").String(getPlayModeString(getPlayMode()).c_str());
    writer.String("packet").Uint(getPacketNumber());
    writer.String("playerPerTeam").Uint(getPlayersPerTeam());
    writer.String("isFirstHalf").Bool(isFirstHalf());
    writer.String("nextKickOffTeamIndex").Uint(getNextKickOffTeamIndex());
    writer.String("isPenaltyShootOut").Bool(isPenaltyShootout());
    writer.String("isOvertime").Bool(isOvertime());
    writer.String("isTimeout").Bool(isTimeout());
    writer.String("lastDropInTeamNum").Uint(getLastDropInTeamNumber());
    writer.String("secSinceDropIn").Int(getSecondsSinceLastDropIn());
    writer.String("secondsRemaining").Int(getSecondsRemaining());
    writer.String("secondsSecondaryTime").Int(getSecondaryTime());

    auto writeTeam = [&writer,this](TeamInfo const& team)
    {
      writer.StartObject();
      {
        writer.String("num").Uint(team.getTeamNumber());
        writer.String("isBlue").Bool(team.isBlueTeam());
        writer.String("score").Uint(team.getScore());
        writer.String("penaltyShotCount").Uint(team.getPenaltyShotCount());

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
                writer.String(getPenaltyTypeString(player.getPenaltyType()).c_str());

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