#pragma once

#include "../typedstateobserver.hh"
#include "../../StateObject/HardwareState/hardwarestate.hh"

#include <Eigen/Core>
#include <Eigen/Geometry>

namespace bold
{
  enum class OrientationTechnique
  {
    Madgwick = 0,
    Sum = 1
  };

  template<typename> class Setting;;

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

    void updateMadgwick(std::shared_ptr<HardwareState const> const& state);

    void updateSum(std::shared_ptr<HardwareState const> const& state);

    // estimated orientation quaternion elements
    float SEq_1, SEq_2, SEq_3, SEq_4;
    Setting<OrientationTechnique>* d_technique;
  };
}
