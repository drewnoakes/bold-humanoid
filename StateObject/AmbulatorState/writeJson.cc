#include "ambulatorstate.hh"

using namespace bold;
using namespace rapidjson;

void AmbulatorState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("target");
    writer.StartArray();
    {
      writer.Double(d_targetX);
      writer.Double(d_targetY);
      writer.Double(d_targetTurn);
    }
    writer.EndArray();

    writer.String("current");
    writer.StartArray();
    {
      writer.Double(d_currentX);
      writer.Double(d_currentY);
      writer.Double(d_currentTurn);
    }
    writer.EndArray();

    writer.String("running").Bool(d_isRunning);

    writer.String("phase").Bool(d_currentPhase);

    writer.String("bodySwingY").Bool(d_bodySwingY);
    writer.String("bodySwingZ").Bool(d_bodySwingZ);
  }
  writer.EndObject();
}