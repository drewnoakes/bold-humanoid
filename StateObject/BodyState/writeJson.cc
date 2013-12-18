#include "bodystate.hh"

using namespace bold;
using namespace rapidjson;

void BodyState::writeJson(Writer<StringBuffer>& writer) const
{
  assert(this);

  writer.StartObject();
  {
    writer.String("cycle").Uint64(d_cycleNumber);

    writer.String("angles");
    writer.StartArray();
    {
      for (unsigned j = (unsigned)JointId::MIN; j <= (unsigned)JointId::MAX; j++)
      {
        auto it = d_jointById.find(j);
        writer.Double(it->second->angle, "%.3f");
      }
    }
    writer.EndArray();

    writer.String("camera-translation");
    writer.StartArray();
    {
      auto translation = d_cameraAgentTransform.translation();
      writer.Double(translation.x());
      writer.Double(translation.y());
      writer.Double(translation.z());
    }
    writer.EndArray();
  }
  writer.EndObject();
}
