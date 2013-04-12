#include "bodystate.hh"

using namespace bold;
using namespace rapidjson;

void BodyState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("angles");
    writer.StartArray();
    {
      for (unsigned j = 1; j < Robot::JointData::NUMBER_OF_JOINTS; j++)
      {
        auto it = d_jointById.find(j);
        writer.Double(it->second->angle);
      }
    }
    writer.EndArray();
  }
  writer.EndObject();
}