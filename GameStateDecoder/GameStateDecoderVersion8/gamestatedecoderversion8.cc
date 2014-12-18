#include "gamestatedecoderversion8.hh"

#include "../../StateObject/GameState/gamestate.hh"

using namespace bold;
using namespace std;

constexpr int PlayerCount = 11;
constexpr int SPLCoachMessageSize = 40;

shared_ptr<GameState const> GameStateDecoderVersion8::decode(BufferReader& reader) const
{
  auto game = make_shared<GameState>();

  game->d_version = 8;
  game->d_packetNumber = reader.readInt8u();
  game->d_playersPerTeam = reader.readInt8u();
  game->d_playMode = (PlayMode)reader.readInt8u();
  game->d_isFirstHalf = (bool)reader.readInt8u();
  game->d_nextKickOffTeamIndex = (bool)reader.readInt8u();
  game->d_periodType = (PeriodType)reader.readInt8u();
  game->d_lastDropInTeamColorNumber = reader.readInt8u();
  game->d_secondsSinceLastDropIn = reader.readInt16u();
  game->d_secondsRemaining = reader.readInt16u();
  game->d_secondaryTime = reader.readInt16u();

  auto readTeam = [&]() -> TeamData
  {
    TeamData team;

    team.d_teamNumber = reader.readInt8u();
    team.d_teamColour = (TeamColor)reader.readInt8u();
    team.d_score = reader.readInt8u();
    team.d_penaltyShot = reader.readInt8u();
    team.d_singleShots = reader.readInt16u();

    // SPL coach
    reader.skip(SPLCoachMessageSize);
    reader.skip(2); // Coach's player data

    vector<PlayerData> players;
    players.resize(game->d_playersPerTeam);
    for (uint8_t i = 0; i < game->d_playersPerTeam; i++)
    {
      const PenaltyType penaltyType = (PenaltyType)reader.readInt8u();
      const uint8_t secondsUntilPenaltyLifted = (uint8_t)reader.readInt8u();
      players[i] = PlayerData(penaltyType, secondsUntilPenaltyLifted);
    }

    team.d_players = move(players);

    reader.skip((size_t)((PlayerCount - game->d_playersPerTeam) * 2));

    return team;
  };

  game->d_team1 = readTeam(); // Blue
  game->d_team2 = readTeam(); // Red

  return game;
}
