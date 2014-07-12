#pragma once

#include "../typedstateobserver.hh"
#include "../../StateObject/GameState/gamestate.hh"

namespace bold
{
  class BehaviourControl;
  class Voice;

  /**
  * Attempts to catch things that the robot handler may have missed, such as a robot being paused
  * as the game is starting, or a penalty period elapsing that the assistant referee has missed.
  */
  class HandlerHelper : public TypedStateObserver<GameState>
  {
  public:
    HandlerHelper(std::shared_ptr<Voice> voice, std::shared_ptr<BehaviourControl> behaviourControl);

    void observeTyped(std::shared_ptr<GameState const> const& state, SequentialTimer& timer) override;

  private:
    std::shared_ptr<Voice> d_voice;
    std::shared_ptr<BehaviourControl> d_behaviourControl;
    Clock::Timestamp d_lastPenaltyLiftAnnounceTime;
    Clock::Timestamp d_lastPauseAtGameStartTime;
  };
}
