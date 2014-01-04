#include "bodystate.hh"

#include <Eigen/Geometry>

using namespace bold;
using namespace Eigen;
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
        writer.Double(it->second->angleRads, "%.3f");
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

    writer.String("camera-rotation");
    writer.StartObject();
    {
      AngleAxisd angleAxis(d_cameraAgentTransform.rotation());
      writer.String("angle").Double(angleAxis.angle());
      writer.String("axis");
      writer.StartArray();
      {
        writer.Double(angleAxis.axis().x());
        writer.Double(angleAxis.axis().y());
        writer.Double(angleAxis.axis().z());
      }
      writer.EndArray();
    }
    writer.EndObject();
  }
  writer.EndObject();
}
