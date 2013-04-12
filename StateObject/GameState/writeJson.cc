#include "gamestate.hh"

using namespace bold;
using namespace rapidjson;

void GameState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("playMode");
    writer.String(getPlayModeString().c_str());

    writer.String("secondsRemaining");
    writer.Int(d_secondsRemaining);
  }
  writer.EndObject();
}