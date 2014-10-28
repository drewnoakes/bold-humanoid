#include "gamestate.hh"

using namespace bold;
using namespace rapidjson;
using namespace robocup;
using namespace std;

GameState::GameState(char const* data)
: d_receivedAt(Clock::getTimestamp())
{
  static_assert(is_pod<GameStateMessage>::value, "Must be POD type");
  static_assert(sizeof(GameStateMessage) == GameStateMessage::SIZE, "Must have advertised size");
  std::copy(data, data + sizeof(GameStateMessage), reinterpret_cast<char*>(&d_data));
}

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

    auto writeTeam = [&writer,this](TeamInfo const& team)
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
