#include "teamstate.hh"

#include "../../Config/config.hh"
#include "../../JsonWriter/jsonwriter.hh"
#include "../../StateObserver/OpenTeamCommunicator/openteamcommunicator.hh"

using namespace bold;
using namespace Eigen;
using namespace rapidjson;
using namespace std;

bool PlayerState::isMe() const
{
  static uchar myUniformNumber = (uchar)Config::getStaticValue<int>("uniform-number");
  static uchar myTeamNumber = (uchar)Config::getStaticValue<int>("team-number");

  return uniformNumber == myUniformNumber && teamNumber == myTeamNumber;
}

double PlayerState::getAgeMillis() const
{
  return Clock::getMillisSince(updateTime);
}

std::ostream& bold::operator<<(std::ostream &stream, PlayerRole const& role)
{
  return stream << getPlayerRoleString(role);
}

std::string bold::getPlayerRoleString(PlayerRole role)
{
  switch (role)
  {
    case PlayerRole::Idle: return "Idle";
    case PlayerRole::Keeper: return "Keeper";
    case PlayerRole::Supporter: return "Supporter";
    case PlayerRole::Striker: return "Striker";
    case PlayerRole::Defender: return "Defender";
    case PlayerRole::PenaltyKeeper: return "PenaltyKeeper";
    case PlayerRole::PenaltyStriker: return "PenaltyStriker";
    case PlayerRole::Other: return "Other";
    case PlayerRole::KickLearner: return "KickLearner";
    case PlayerRole::BallCircler: return "BallCircler";
    case PlayerRole::WhistleListener: return "WhistleListener";
    default: return "Unknown";
  }
}

std::string bold::getPlayerActivityString(PlayerActivity activity)
{
  switch (activity)
  {
    case PlayerActivity::Positioning: return "Positioning";
    case PlayerActivity::ApproachingBall: return "ApproachingBall";
    case PlayerActivity::AttackingGoal: return "AttackingGoal";
    case PlayerActivity::Waiting: return "Waiting";
    case PlayerActivity::Other: return "Other";
    default: return "Unknown";
  }
}

std::string bold::getPlayerStatusString(PlayerStatus status)
{
  switch (status)
  {
    case PlayerStatus::Inactive: return "Inactive";
    case PlayerStatus::Active: return "Active";
    case PlayerStatus::Penalised: return "Penalised";
    case PlayerStatus::Paused: return "Paused";
    default: return "Unknown";
  }
}

vector<PlayerState> TeamState::getBallObservers() const
{
  vector<PlayerState> observers = {};

  for (PlayerState const& player : d_playerStates)
  {
    if (player.isMe())
      continue;

    // TODO review this threshold
    if (Clock::getMillisSince(player.updateTime) > 5000)
      continue;

    if (!player.ballRelative.hasValue())
      continue;

    if (
//       player.status == PlayerStatus::Inactive ||
      player.status == PlayerStatus::Penalised)
    {
      continue;
    }

    // TODO if the ball is *right* in front of the keeper, let the keeper kick it away...
    if (player.role == PlayerRole::Keeper)
      continue;

    observers.push_back(player);
  }

  return observers;
}

bool TeamState::isTeamMateInActivity(PlayerActivity activity) const
{
  for (PlayerState const& player : d_playerStates)
  {
    if (player.isMe())
      continue;

    // TODO review this threshold
    if (Clock::getMillisSince(player.updateTime) > 5000)
      continue;

    if (
//       player.status == PlayerStatus::Inactive ||
      player.status == PlayerStatus::Penalised)
      continue;

    if (player.activity == activity)
      return true;
  }

  return false;
}

FieldSide TeamState::getKeeperBallSideEstimate() const
{
  auto keeper = getKeeperState();

  // If we see a goal, and have ball observation data from the keeper which is recent enough
  if (keeper == nullptr || keeper->getAgeMillis() > 10000 || !keeper->ballRelative.hasValue())
    return FieldSide::Unknown;

  // ASSUME the keeper is roughly in the middle of the penalty area

  double keeperDistFromGoalLine = FieldMap::getGoalAreaLengthX() / 2.0;

  const double maxPositionMeasurementError = 0.4; // TODO review this experimentally

  double keeperBallDist = keeper->ballRelative->norm();

  // Distance from the keeper within which the ball is unambiguously on our side of the field
  double oursThreshold = (FieldMap::getFieldLengthX() / 2.0) - keeperDistFromGoalLine - maxPositionMeasurementError;

  if (keeperBallDist < oursThreshold)
    return FieldSide::Ours;

  // Distance from the keeper above which the ball is unambiguously on the opponent's side of the field
  double theirsThreshold = Vector2d(
    (FieldMap::getFieldLengthX() / 2.0) - keeperDistFromGoalLine + maxPositionMeasurementError,
    FieldMap::getFieldLengthY() / 2.0
  ).norm();

  if (keeperBallDist > theirsThreshold)
    return FieldSide::Theirs;

  return FieldSide::Unknown;
}
