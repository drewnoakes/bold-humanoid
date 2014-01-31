#pragma once

#include "../typedstateobserver.hh"
#include "../../StateObject/HardwareState/hardwarestate.hh"

#include <Eigen/Core>
#include <Eigen/Geometry>

namespace bold
{
  /** Tracks the orientation of the torso using data from the IMU.
   */
  class OrientationTracker : public TypedStateObserver<HardwareState>
  {
  public:
    OrientationTracker();

    Eigen::Quaterniond getQuaternion() const;

    void reset();

  private:
    void observeTyped(std::shared_ptr<HardwareState const> const& state, SequentialTimer& timer) override;

    void filterUpdate(Eigen::Vector3d const& gyro, Eigen::Vector3d const& acc);

    // estimated orientation quaternion elements
    float SEq_1, SEq_2, SEq_3, SEq_4;
  };
}
