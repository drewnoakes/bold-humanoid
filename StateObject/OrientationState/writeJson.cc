#include "orientationstate.hh"

using namespace bold;
using namespace Eigen;

void OrientationState::writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("quaternion");
    writer.StartArray();
    writer.Double(d_quaternion.w(), "%.5f");
    writer.Double(d_quaternion.x(), "%.5f");
    writer.Double(d_quaternion.y(), "%.5f");
    writer.Double(d_quaternion.z(), "%.5f");
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
