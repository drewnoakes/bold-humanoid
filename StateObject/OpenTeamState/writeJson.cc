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
          writer.String("state").Uint(teamMate.data[ROBOT_CURRENT_STATE]);
          writer.String("role").Uint(teamMate.data[ROBOT_CURRENT_ROLE]);
          writer.String("x").Uint(teamMate.data[ROBOT_ABSOLUTE_X]);
          writer.String("y").Uint(teamMate.data[ROBOT_ABSOLUTE_Y]);
          writer.String("theta").Uint(teamMate.data[ROBOT_ABSOLUTE_ORIENTATION]);
          writer.String("ballX").Uint(teamMate.data[BALL_RELATIVE_X]);
          writer.String("ballY").Uint(teamMate.data[BALL_RELATIVE_Y]);
        }
        writer.EndObject();
      }
    }
    writer.EndArray();
  }
  writer.EndObject();
}
