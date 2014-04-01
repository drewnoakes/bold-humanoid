#pragma once

#include "../typedstateobserver.hh"
#include "../../StateObject/BodyState/bodystate.hh"

namespace bold
{
  class WalkModule;

  /** Calculates cumulative translation of the torso over time.
   *
   * Done by observing HardwareState and knowledge of the stance foot.
   */
  class Odometer : public TypedStateObserver<BodyState>
  {
  public:
    explicit Odometer(std::shared_ptr<WalkModule> walkModule);

    Eigen::Vector3d getTranslation() const;

  private:
    void observeTyped(std::shared_ptr<BodyState const> const& state, SequentialTimer& timer) override;

    std::shared_ptr<WalkModule> const d_walkModule;
    std::shared_ptr<BodyState const> d_lastBodyState;
    Eigen::Vector3d d_progress;
    mutable std::mutex d_progressMutex;
  };
}
