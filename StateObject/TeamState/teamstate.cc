#include "teamstate.hh"

#include "../../Config/config.hh"
#include "../../StateObserver/OpenTeamCommunicator/openteamcommunicator.hh"

using namespace bold;
using namespace rapidjson;
using namespace std;

void TeamState::writeJson(Writer<StringBuffer>& writer) const
{
  auto swapNaN = [](double d, double nanVal) -> double { return std::isnan(d) ? nanVal : d; };

  writer.StartObject();
  {
    writer.String("players");
    writer.StartArray();
    {
      for (PlayerState const& player : d_playerStates)
      {
        writer.StartObject();
        {
          writer.String("unum").Uint(player.uniformNumber);
          writer.String("team").Int(player.teamNumber);
          writer.String("isMe").Bool(player.isMe());
          writer.String("activity").Int(static_cast<int>(player.activity));
          writer.String("status").Int(static_cast<int>(player.status));
          writer.String("role").Int(static_cast<int>(player.role));
          writer.String("pos")
            .StartArray()
              .Double(swapNaN(player.pos.x(), 0))
              .Double(swapNaN(player.pos.y(), 0))
              .Double(swapNaN(player.pos.theta(), 0))
            .EndArray();
          writer.String("posConfidence").Double(player.posConfidence);
          writer.String("ballRelative").StartArray();
          {
            if (player.ballRelative.hasValue())
              writer.Double(player.ballRelative->x()).Double(player.ballRelative->y());
          }
          writer.EndArray();
          writer.String("updateTime").Uint64(Clock::timestampToMillis(player.updateTime));
        }
        writer.EndObject();
      }
    }
    writer.EndArray();
  }
  writer.EndObject();
}

bool PlayerState::isMe() const
{
  static int myUniformNumber = Config::getStaticValue<int>("uniform-number");
  static int myTeamNumber = Config::getStaticValue<int>("team-number");

  return uniformNumber == myUniformNumber && teamNumber == myTeamNumber;
}

double PlayerState::getAgeMillis() const
{
  return Clock::getMillisSince(updateTime);
}

std::ostream& bold::operator<<(std::ostream &stream, PlayerRole const& role)
{
  switch (role)
  {
    case PlayerRole::Idle: stream << "Idle"; break;
    case PlayerRole::Keeper: stream << "Keeper"; break;
    case PlayerRole::Supporter: stream << "Supporter"; break;
    case PlayerRole::Striker: stream << "Striker"; break;
    case PlayerRole::Defender: stream << "Defender"; break;
    case PlayerRole::PenaltyKeeper: stream << "PenaltyKeeper"; break;
    case PlayerRole::PenaltyStriker: stream << "PenaltyStriker"; break;
    case PlayerRole::Other: stream << "Other"; break;
    default: stream << "Unknown " << (int)role; break;
  }
  return stream;
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

bool TeamState::isTeamMate(PlayerActivity activity) const
{
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
      continue;

    if (player.activity == activity)
      return true;
  }

  return false;
}
