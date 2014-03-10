#include "openteamstate.hh"

#include "../../StateObserver/OpenTeamCommunicator/openteamcommunicator.hh"

using namespace bold;
using namespace rapidjson;
using namespace std;

void OpenTeamState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("teammates");
    writer.StartArray();
    {
      for (auto const& pair : d_teamMates)
      {
        MixedTeamMate teamMate = pair.second;
        writer.StartObject();
        {
          writer.String("lastUpdate").Uint(Clock::timestampToMillis(teamMate.lastUpdate));
          writer.String("robotId").Uint(teamMate.robotID);
          writer.String("state").Int(teamMate.data[ROBOT_CURRENT_STATE]);
          writer.String("role").Int(teamMate.data[ROBOT_CURRENT_ROLE]);
          writer.String("x").Int(teamMate.data[ROBOT_ABSOLUTE_X]);
          writer.String("y").Int(teamMate.data[ROBOT_ABSOLUTE_Y]);
          writer.String("theta").Int(teamMate.data[ROBOT_ABSOLUTE_ORIENTATION]);
          writer.String("ballX").Int(teamMate.data[BALL_RELATIVE_X]);
          writer.String("ballY").Int(teamMate.data[BALL_RELATIVE_Y]);
        }
        writer.EndObject();
      }
    }
    writer.EndArray();
  }
  writer.EndObject();
}
