#pragma once

#include "../../Config/config.hh"
#include "../../GameStateReceiver/gamecontrollertypes.hh"
#include "../../State/state.hh"
#include "../../StateObject/GameState/gamestate.hh"
#include "../../util/conditionals.hh"

namespace bold
{
  //
  // Utility functions for setting status flags as we enter states
  //

  auto setPlayerActivityInStates = [](Agent* agent, PlayerActivity activity, std::vector<shared_ptr<FSMState>> states)
  {
    for (shared_ptr<FSMState>& state : states)
      state->onEnter.connect([agent,activity]() { agent->getBehaviourControl()->setPlayerActivity(activity); });
  };

  auto setPlayerStatusInStates = [](Agent* agent, PlayerStatus status, std::vector<shared_ptr<FSMState>> states)
  {
    for (shared_ptr<FSMState>& state : states)
      state->onEnter.connect([agent,status]() { agent->getBehaviourControl()->setPlayerStatus(status); });
  };


  //
  // CONDITIONALS
  //

  // TODO any / all / true functions

  // GENERAL FUNCTIONS

  auto startButtonPressed = []()
  {
    auto hw = State::get<HardwareState>();
    if (!hw)
      return false;
    auto const& cm730 = hw->getCM730State();

    static bool lastState = false;
    if (lastState ^ cm730.isStartButtonPressed)
    {
      lastState = cm730.isStartButtonPressed;
      return lastState;
    }

    return false;
  };

  auto modeButtonPressed = []()
  {
    auto hw = State::get<HardwareState>();
    if (!hw)
      return false;
    auto const& cm730 = hw->getCM730State();

    static bool lastState = false;
    if (lastState ^ cm730.isModeButtonPressed)
    {
      lastState = cm730.isModeButtonPressed;
      return lastState;
    }

    return false;
  };

  auto ballVisibleCondition = []()
  {
    return State::get<CameraFrameState>()->isBallVisible();
  };

  auto ballTooFarToKick = []()
  {
    // TODO use filtered ball position
    auto ballObs = State::get<AgentFrameState>()->getBallObservation();
    static auto maxKickDistance = Config::getSetting<double>("kick.max-ball-distance");
    return ballObs && ballObs->head<2>().norm() > maxKickDistance->getValue();
  };

  // TODO review this one-size-fits-all approach on a case-by-case basis below
  auto ballFoundConditionFactory = []() { return trueForMillis(1000, ballVisibleCondition); };
  auto ballLostConditionFactory = []() { return trueForMillis(1000, bold::negate(ballVisibleCondition)); };

  auto isPlayMode = [](robocup::PlayMode playMode, bool defaultValue)
  {
    return [playMode,defaultValue]()
    {
      auto gameState = State::get<GameState>();
      if (!gameState)
        return defaultValue;
      return gameState->getPlayMode() == playMode;
    };
  };

  auto isSetPlayMode = isPlayMode(robocup::PlayMode::SET, false);
  auto isPlayingPlayMode = isPlayMode(robocup::PlayMode::PLAYING, false);
}
