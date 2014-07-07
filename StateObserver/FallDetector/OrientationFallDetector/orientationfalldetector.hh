#pragma once

#include "../falldetector.hh"

#include "../../typedstateobserver.hh"
#include "../../../StateObject/OrientationState/orientationstate.hh"

namespace bold
{
  class OrientationFallDetector : public FallDetector, public TypedStateObserver<OrientationState>
  {
  public:
    OrientationFallDetector(std::shared_ptr<Voice> voice);

    void observeTyped(std::shared_ptr<OrientationState const> const& hardwareState, SequentialTimer& timer) override;

  protected:
    void logFallData(std::stringstream& msg) const override;
  };
}
