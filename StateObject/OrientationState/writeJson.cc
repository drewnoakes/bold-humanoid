#include "orientationstate.hh"

#include "../../Math/math.hh"

using namespace bold;
using namespace Eigen;
using namespace rapidjson;

double OrientationState::getYawAngle() const
{
  return Math::normaliseRads(Math::yawFromQuaternion(d_quaternion));
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

    /*
    writer.String("angle-axis");
    AngleAxisd aa(d_quaternion);
    writer.StartArray();
    writer.Double(aa.angle());
    writer.Double(aa.axis().x());
    writer.Double(aa.axis().y());
    writer.Double(aa.axis().z());
    writer.EndArray();

    writer.String("euler");
    Affine3d af(d_quaternion);
    Matrix3d m(af.matrix().block<3,3>(0, 0));
    writer.StartArray();
    writer.Double(m(0));
    writer.Double(m(1));
    writer.Double(m(2));
    writer.Double(m(3));
    writer.Double(m(4));
    writer.Double(m(5));
    writer.Double(m(6));
    writer.Double(m(7));
    writer.Double(m(8));
    writer.EndArray();
    */
  }
  writer.EndObject();
}
