#include "roledecider.hh"

#include "../Config/config.hh"
#include "../BehaviourControl/behaviourcontrol.hh"
#include "../Debugger/debugger.hh"
#include "../State/state.hh"
#include "../StateObject/AgentFrameState/agentframestate.hh"
#include "../StateObject/TeamState/teamstate.hh"

#include <limits>

using namespace bold;

void RoleDecider::update()
{
  /*
    Decide between the following PlayerRole values.

    Keeper    -- robot is acting as the keeper
    Supporter -- robot is positioning to receive a pass towards the goal
    Striker   -- robot is claiming possession of the ball and advancing it towards the opponent's goal
    Defender  -- robot is positioning so as to block an opponent's advance towards our goal

    PenaltyKeeper  -- robot is acting as a keeper during a penalty shootout
    PenaltyStriker -- robot is acting as a striker during a penalty shootout
  */

  static int uniformNumber = Config::getStaticValue<int>("uniform-number");

  auto setRole = [this](PlayerRole role)
  {
    d_debugger->showRole(role);
    d_behaviourControl->setPlayerRole(role);
  };

  if (uniformNumber == GOALIE_UNUM)
  {
    setRole(PlayerRole::Keeper);
    return;
  }

//   if (uniformNumber == 5)
//   {
//     setRole(PlayerRole::PenaltyKeeper);
//     return;
//   }
//
//   if (uniformNumber == 6)
//   {
//     setRole(PlayerRole::PenaltyStriker);
//     return;
//   }

  //
  // Decide between: striker, supporter & defender
  //

  auto agentFrame = State::get<AgentFrameState>();

  if (!agentFrame || !agentFrame->getBallObservation().hasValue())
  {
    // TODO if we cannot see the ball, use info from teammates to determine our role
    setRole(PlayerRole::Striker);
    return;
  }

  auto teamState = State::get<TeamState>();

  if (!teamState || teamState->empty())
  {
    // We have no information about our teammates' position or roles, so default
    // to being a striker.
    setRole(PlayerRole::Striker);
    return;
  }

  // TODO if the keeper is closest to the ball, and it's within a certain area in front of the bot, turn into a striker and kick it away

  // TODO if no one observed as a striker for a certain period of time, become one automatically

  // TODO discard teamState if last update time > some threshold

  //
  // Find who is the closest to the ball
  //

  double closestDistance = std::numeric_limits<double>::max();

  for (PlayerState const& player : teamState->getBallObservers())
  {
    double dist = player.ballRelative->norm();

    if (dist < closestDistance)
      closestDistance = dist;
  }

  double dist = agentFrame->getBallObservation()->norm();

  // If I am closest, become the striker, otherwise support
  setRole(dist < closestDistance ? PlayerRole::Striker : PlayerRole::Supporter);
}
