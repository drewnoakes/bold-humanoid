#include "orientationstate.hh"

#include "../../Math/math.hh"

using namespace bold;
using namespace Eigen;
using namespace rapidjson;

OrientationState::OrientationState(Eigen::Quaterniond quaternion)
  : d_quaternion(quaternion),
    d_pitch(Math::normaliseRads(Math::pitchFromQuaternion(quaternion)),
    d_roll(Math::normaliseRads(Math::rollFromQuaternion(quaternion)),
    d_yaw(Math::normaliseRads(Math::yawFromQuaternion(quaternion))
{}

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

    auto const& q1 = d_quaternion;

    double sqw = q1.w()*q1.w();
    double sqx = q1.x()*q1.x();
    double sqy = q1.y()*q1.y();
    double sqz = q1.z()*q1.z();
    double unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
    double test = q1.x()*q1.y() + q1.z()*q1.w();

    double heading;
    double attitude;
    double bank;

    if (test > 0.499*unit) { // singularity at north pole
      heading = 2 * atan2(q1.x(),q1.w());
      attitude = M_PI/2;
      bank = 0;
    }
    else if (test < -0.499*unit) { // singularity at south pole
      heading = -2 * atan2(q1.x(),q1.w());
      attitude = -M_PI/2;
      bank = 0;
    }
    else
    {
      heading = atan2(2*q1.y()*q1.w()-2*q1.x()*q1.z() , sqx - sqy - sqz + sqw);
      attitude = asin(2*test/unit);
      bank = atan2(2*q1.x()*q1.w()-2*q1.y()*q1.z() , -sqx + sqy - sqz + sqw);
    }

    writer.String("pitch").Double(attitude);
    writer.String("roll").Double(bank);
    writer.String("yaw").Double(heading);
//    writer.String("pitch").Double(Math::pitchFromQuaternion(d_quaternion));
//    writer.String("roll").Double(Math::rollFromQuaternion(d_quaternion));
//    writer.String("yaw").Double(Math::yawFromQuaternion(d_quaternion));

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
