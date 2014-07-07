#include "orientationstate.hh"

#include "../../Math/math.hh"

#include <iomanip>

using namespace bold;
using namespace Eigen;
using namespace rapidjson;

OrientationState::OrientationState(Quaterniond const& quaternion)
: d_quaternion(quaternion)
{
  ASSERT(quaternion.w() != 0 || quaternion.x() != 0 || quaternion.y() != 0 || quaternion.z() != 0);

  Affine3d rotation(d_quaternion);
  Vector3d axisX1 = rotation.matrix().col(0).head<3>();
  d_yaw = atan2(axisX1.y(), axisX1.x());

  rotation = AngleAxisd(-d_yaw, Vector3d::UnitZ()) * rotation;

  Vector3d axisY = rotation.matrix().col(1).head<3>();
  d_pitch = atan2(axisY.z(), axisY.y());

  rotation = AngleAxisd(-d_pitch, Vector3d::UnitX()) * rotation;

  Vector3d axisX2 = rotation.matrix().col(0).head<3>();
  d_roll = atan2(axisX2.z(), axisX2.x());
}

void OrientationState::writeJson(Writer<StringBuffer>& writer) const
{
  auto swapNaN = [](double d, double nanVal) -> double { return std::isnan(d) ? nanVal : d; };

  writer.StartObject();
  {
    writer.String("quaternion");
    writer.StartArray();
    writer.Double(swapNaN(d_quaternion.x(), 0), "%.5f");
    writer.Double(swapNaN(d_quaternion.y(), 0), "%.5f");
    writer.Double(swapNaN(d_quaternion.z(), 0), "%.5f");
    writer.Double(swapNaN(d_quaternion.w(), 0), "%.5f");
    writer.EndArray();

    writer.String("pitch").Double(swapNaN(d_pitch, 0));
    writer.String("roll").Double(swapNaN(d_roll, 0));
    writer.String("yaw").Double(swapNaN(d_yaw, 0));
  }
  writer.EndObject();
}
