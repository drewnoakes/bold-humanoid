#include "gamestate.hh"

using namespace bold;
using namespace rapidjson;
using namespace robocup;
using namespace std;

GameState::GameState(char const* data)
: d_receivedAt(Clock::getTimestamp())
{
  static_assert(is_pod<GameStateMessage>::value, "Must be POD type");
  static_assert(sizeof(PlayerInfo) == PlayerInfo::SIZE, "Must have advertised size");
  static_assert(sizeof(TeamInfo) == TeamInfo::SIZE, "Must have advertised size");
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
