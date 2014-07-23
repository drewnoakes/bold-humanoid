#include "orientationstate.hh"

#include "../../Math/math.hh"
#include "../../JsonWriter/jsonwriter.hh"

#include <iomanip>

using namespace bold;
using namespace Eigen;
using namespace rapidjson;

OrientationState::OrientationState(Quaterniond const& quaternion)
: d_quaternion(quaternion)
{
  ASSERT(quaternion.w() != 0 || quaternion.x() != 0 || quaternion.y() != 0 || quaternion.z() != 0);
  ASSERT(!::isnan(quaternion.w()));
  ASSERT(!::isnan(quaternion.x()));
  ASSERT(!::isnan(quaternion.y()));
  ASSERT(!::isnan(quaternion.z()));

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

Affine3d OrientationState::withoutYaw() const
{
  return AngleAxisd(-d_yaw, Vector3d::UnitZ()) * Affine3d(d_quaternion);
}

void OrientationState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("quaternion");
    writer.StartArray();
    JsonWriter::swapNaN(writer, d_quaternion.x());
    JsonWriter::swapNaN(writer, d_quaternion.y());
    JsonWriter::swapNaN(writer, d_quaternion.z());
    JsonWriter::swapNaN(writer, d_quaternion.w());
    writer.EndArray();

    writer.String("pitch"); JsonWriter::swapNaN(writer, d_pitch);
    writer.String("roll");  JsonWriter::swapNaN(writer, d_roll);
    writer.String("yaw");   JsonWriter::swapNaN(writer, d_yaw);
  }
  writer.EndObject();
}
