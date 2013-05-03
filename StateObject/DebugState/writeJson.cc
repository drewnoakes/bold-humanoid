#include "debugstate.hh"

using namespace std;
using namespace bold;
using namespace rapidjson;

void DebugState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("timings");
    writer.StartObject();
    {
      vector<EventTiming> const& timings = *d_eventTimings;
      for (EventTiming const& timing : timings)
      {
        writer.String(timing.second.c_str());
        writer.Double(timing.first * 1000.0);
      }
    }
    writer.EndObject();

    writer.String("gameControllerMessages");
    writer.Int(d_gameControllerMessageCount);

    writer.String("ignoredMessages");
    writer.Int(d_ignoredMessageCount);
  }
  writer.EndObject();
}