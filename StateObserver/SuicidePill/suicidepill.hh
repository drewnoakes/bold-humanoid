#pragma once

#include "../stateobserver.hh"
#include "../../Clock/clock.hh"
#include "../../StateObject/HardwareState/hardwarestate.hh"

namespace bold
{
  class Agent;
  class Debugger;

  /** Kills the agent when requested by the robot handler.
   *
   * Holding both the start and mode button down for N seconds will exit the
   * process. Holding them both for M (M > N) seconds will stop the
   * boldhumanoid service.
   */
  class SuicidePill : public TypedStateObserver<HardwareState>
  {
  public:
    SuicidePill(Agent* agent, std::shared_ptr<Debugger> debugger);

  private:
    void observeTyped(std::shared_ptr<HardwareState const> state) override;

    Agent* const d_agent;
    const std::shared_ptr<Debugger> d_debugger;
    bool d_isActive;
    Clock::Timestamp d_pressedAt;
    bool d_exited;
  };
}
