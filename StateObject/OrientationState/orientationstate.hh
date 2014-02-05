#pragma once

#include "../stateobject.hh"

#include <Eigen/Geometry>

namespace bold
{
  class OrientationState : public StateObject
  {
  public:
    OrientationState(Eigen::Quaterniond quaternion)
    : d_quaternion(quaternion)
    {}

    Eigen::Quaterniond const& getQuaternion() const { return d_quaternion; };

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

  private:
    Eigen::Quaterniond d_quaternion;
  };
}
