#pragma once

#include "../stateobject.hh"

#include <Eigen/Geometry>

namespace bold
{
  class OrientationState : public StateObject
  {
  public:
    OrientationState(Eigen::Quaterniond const& quaternion);

    Eigen::Quaterniond const& getQuaternion() const { return d_quaternion; };

    double getPitchAngle() const { return d_pitch; }
    double getRollAngle() const { return d_roll; }
    double getYawAngle() const { return d_yaw; }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

  private:
    Eigen::Quaterniond d_quaternion;
    double d_pitch;
    double d_roll;
    double d_yaw;
  };
}
