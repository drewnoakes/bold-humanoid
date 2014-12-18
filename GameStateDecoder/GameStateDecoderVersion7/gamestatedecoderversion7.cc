#include "gamestatedecoderversion7.hh"

#include "../../StateObject/GameState/gamestate.hh"

using namespace bold;
using namespace std;

constexpr int PlayerCount = 11;

shared_ptr<GameState const> GameStateDecoderVersion7::decode(BufferReader& reader) const
{
  auto game = make_shared<GameState>();

  // In version 7, the packet number was 32 bit. The caller reads the first only (LSB), so
  // we must skip the remaining three.
  reader.skip(3);

  game->d_version = 7;
  game->d_packetNumber = 0;
  game->d_secondaryTime = 0;

  game->d_playersPerTeam = reader.readInt8u();
  game->d_playMode = (PlayMode)reader.readInt8u();
  game->d_isFirstHalf = (bool)reader.readInt8u();
  game->d_nextKickOffTeamIndex = (bool)reader.readInt8u();
  game->d_periodType = (PeriodType)reader.readInt8u();
  game->d_lastDropInTeamColorNumber = reader.readInt8u();
  game->d_secondsSinceLastDropIn = reader.readInt16u();
  game->d_secondsRemaining = (int16_t) reader.readInt32u();

  auto readTeam = [&]() -> TeamData
  {
    TeamData team;

    team.d_penaltyShot = 0;
    team.d_singleShots = 0;

    team.d_teamNumber = reader.readInt8u();
    team.d_teamColour = (TeamColor)reader.readInt8u();

    // Skip the goal colour, as goals are the same colour nowadays
    reader.skip(1);

    team.d_score = reader.readInt8u();

    vector<PlayerData> players;
    players.resize(game->d_playersPerTeam);
    for (uint8_t i = 0; i < game->d_playersPerTeam; i++)
    {
      const PenaltyType penaltyType = (PenaltyType)reader.readInt16u();
      const uint8_t secondsUntilPenaltyLifted = (uint8_t)reader.readInt16u();
      players[i] = PlayerData(penaltyType, secondsUntilPenaltyLifted);
    }
    team.d_players = move(players);

    const size_t skipCount = (size_t)((PlayerCount - game->d_playersPerTeam) * 4);
    reader.skip(skipCount);

    return team;
  };

  game->d_team1 = readTeam(); // Blue
  game->d_team2 = readTeam(); // Red

  return game;
}
