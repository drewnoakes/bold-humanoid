#pragma once

#include "../../BehaviourControl/behaviourcontrol.hh"
#include "../../Config/config.hh"
#include "../../Option/FSMOption/fsmoption.hh"
#include "../../State/state.hh"
#include "../../StateObject/AgentFrameState/agentframestate.hh"
#include "../../StateObject/CameraFrameState/cameraframestate.hh"
#include "../../StateObject/GameState/gamestate.hh"
#include "../../StateObject/TeamState/teamstate.hh"
#include "../../util/conditionals.hh"

namespace bold
{
  //
  // Utility functions for setting status flags as we enter states
  //

  auto setPlayerActivityInStates = [](Agent* agent, PlayerActivity activity, std::vector<std::shared_ptr<FSMState>> states)
  {
    for (std::shared_ptr<FSMState>& state : states)
      state->onEnter.connect([agent,activity] { agent->getBehaviourControl()->setPlayerActivity(activity); });
  };

  auto setPlayerStatusInStates = [](Agent* agent, PlayerStatus status, std::vector<std::shared_ptr<FSMState>> states)
  {
    for (std::shared_ptr<FSMState>& state : states)
      state->onEnter.connect([agent,status] { agent->getBehaviourControl()->setPlayerStatus(status); });
  };

  //
  // CONDITIONALS
  //

  // TODO any / all / true functions

  // GENERAL FUNCTIONS

  auto ballVisibleCondition = []
  {
    return State::get<CameraFrameState>()->isBallVisible();
  };

  auto ballNotVisibleCondition = []
  {
    return State::get<CameraFrameState>()->isBallVisible();
  };

  auto ballTooFarToKick = []
  {
    // TODO use filtered ball position
    auto ballObs = State::get<AgentFrameState>()->getBallObservation();
    static auto maxKickDistance = Config::getSetting<double>("kick.max-ball-distance");
    return ballObs && ballObs->head<2>().norm() > maxKickDistance->getValue();
  };

  // TODO review this one-size-fits-all approach on a case-by-case basis below
  auto ballFoundConditionFactory = [] { return trueForMillis(1000, ballVisibleCondition); };
  auto ballLostConditionFactory = [] { return trueForMillis(1000, bold::negate(ballVisibleCondition)); };

  auto isPenalised = []
  {
    auto gameState = State::get<GameState>();
    return gameState && gameState->getMyPlayerInfo().hasPenalty();
  };

  auto nonPenalisedPlayMode = [](PlayMode playMode)
  {
    return [playMode]
    {
      auto gameState = State::get<GameState>();
      return gameState && !gameState->getMyPlayerInfo().hasPenalty() && gameState->getPlayMode() == playMode;
    };
  };

  auto ballIsStoppingDistance = []
  {
    // Approach ball until we're within a given distance
    // TODO use filtered ball position
    auto ballObs = State::get<AgentFrameState>()->getBallObservation();
    static auto stoppingDistance = Config::getSetting<double>("options.approach-ball.stop-distance");
    return ballObs && (ballObs->head<2>().norm() < stoppingDistance->getValue());
  };
}
