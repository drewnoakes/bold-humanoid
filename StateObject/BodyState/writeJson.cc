#include "bodystate.hh"

#include <Eigen/Geometry>

using namespace bold;
using namespace Eigen;
using namespace rapidjson;

void BodyState::writeJson(Writer<StringBuffer>& writer) const
{
  ASSERT(this);

  writer.StartObject();
  {
    writer.String("motion-cycle").Uint64(d_motionCycleNumber);

    writer.String("angles");
    writer.StartArray();
    {
      for (uchar j = (uchar)JointId::MIN; j <= (uchar)JointId::MAX; j++)
      {
        auto const& it = d_jointById[j];
        writer.Double(it->getAngleRads(), "%.3f");
      }
    }
    writer.EndArray();

    writer.String("errors");
    writer.StartArray();
    {
      for (int diff : d_positionValueDiffs)
        writer.Int(diff);
    }
    writer.EndArray();

    /*
    writer.String("camera-translation");
    writer.StartArray();
    {
      auto translation = d_agentCameraTransform.translation();
      writer.Double(translation.x());
      writer.Double(translation.y());
      writer.Double(translation.z());
    }
    writer.EndArray();

    writer.String("camera-rotation");
    writer.StartObject();
    {
      AngleAxisd angleAxis(d_agentCameraTransform.rotation());
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
    */
  }
  writer.EndObject();
}
