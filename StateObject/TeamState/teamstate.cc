#include "teamstate.hh"

#include "../../Config/config.hh"
#include "../../StateObserver/OpenTeamCommunicator/openteamcommunicator.hh"

using namespace bold;
using namespace rapidjson;
using namespace std;

void TeamState::writeJson(Writer<StringBuffer>& writer) const
{
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
              .Double(player.pos.x())
              .Double(player.pos.y())
              .Double(player.pos.theta())
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
