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
      writer.Double(d_targetX, "%.2f");
      writer.Double(d_targetY, "%.2f");
      writer.Double(d_targetTurn, "%.2f");
    }
    writer.EndArray();

    writer.String("current");
    writer.StartArray();
    {
      writer.Double(d_currentX, "%.2f");
      writer.Double(d_currentY, "%.2f");
      writer.Double(d_currentTurn, "%.2f");
    }
    writer.EndArray();

    writer.String("delta");
    writer.StartArray();
    {
      writer.Double(d_lastXDelta, "%.2f");
      writer.Double(d_lastYDelta, "%.2f");
      writer.Double(d_lastTurnDelta, "%.2f");
    }
    writer.EndArray();

    writer.String("running").Bool(d_isRunning);

    writer.String("phase").Int(d_currentPhase);

    writer.String("hipPitch").Double(d_hipPitch, "%.3f");
    writer.String("bodySwingY").Double(d_bodySwingY, "%.3f");
    writer.String("bodySwingZ").Double(d_bodySwingZ, "%.3f");
  }
  writer.EndObject();
}
