#pragma once

#include "../typedstateobserver.hh"
#include "../../StateObject/BodyState/bodystate.hh"

namespace bold
{
  class WalkModule;

  class Odometer : public TypedStateObserver<BodyState>
  {
  public:
    explicit Odometer(std::shared_ptr<WalkModule> walkModule);

    Eigen::Vector3d flush();

  private:
    void observeTyped(std::shared_ptr<BodyState const> const& state, SequentialTimer& timer) override;

    std::shared_ptr<WalkModule> const d_walkModule;
    std::shared_ptr<BodyState const> d_lastBodyState;
    Eigen::Vector3d d_progress;
    std::mutex d_progressMutex;
  };
}
