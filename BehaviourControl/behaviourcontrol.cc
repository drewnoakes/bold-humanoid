#include "behaviourcontrol.hh"

#include "../Agent/agent.hh"
#include "../State/state.hh"
#include "../StateObject/BehaviourControlState/behaviourcontrolstate.hh"

using namespace bold;
using namespace std;

BehaviourControl::BehaviourControl(Agent& agent)
: d_agent(agent),
  d_playerRoleSetThisCycle(false),
  d_playerStatusSetThisCycle(false),
  d_playerActivitySetThisCycle(false),
  d_playModeSetThisCycle(false)
{
}

void BehaviourControl::updateStateObject()
{
  d_playerRoleSetThisCycle = false;
  d_playerStatusSetThisCycle = false;
  d_playerActivitySetThisCycle = false;
  d_playModeSetThisCycle = false;
  State::make<BehaviourControlState>(d_agent.getThinkCycleNumber(), d_playerRole, d_playerActivity, d_playerStatus);
}

void BehaviourControl::setPlayerRole(PlayerRole role)
{
  if (role == d_playerRole)
    return;
  if (d_playerRoleSetThisCycle)
    log::warning("BehaviourControl::setPlayerRole") << "PlayerRole set multiple times during think cycle, with values " << getPlayerRoleString(d_playerRole) << " and " << getPlayerRoleString(role);
  d_playerRole = role;
  d_playerRoleSetThisCycle = true;
}

void BehaviourControl::setPlayerStatus(PlayerStatus status)
{
  if (status == d_playerStatus)
    return;
  if (d_playerStatusSetThisCycle)
  {
    log::warning("BehaviourControl::setPlayerStatus") << "PlayerStatus set multiple times during think cycle, with values " << getPlayerStatusString(d_playerStatus) << " and " << getPlayerStatusString(status);
    ASSERT(false);
  }
  d_playerStatus = status;
  d_playerStatusSetThisCycle = true;
}

void BehaviourControl::setPlayerActivity(PlayerActivity activity)
{
  if (activity == d_playerActivity)
    return;
  if (d_playerActivitySetThisCycle)
  {
    log::warning("BehaviourControl::setPlayerActivity") << "PlayerActivity set multiple times during think cycle, with values " << getPlayerActivityString(d_playerActivity) << " and " << getPlayerActivityString(activity);
    ASSERT(false);
  }
  d_playerActivity = activity;
  d_playerActivitySetThisCycle = true;
}

void BehaviourControl::setPlayMode(PlayMode playMode)
{
  if (playMode == d_playMode)
    return;
  if (d_playModeSetThisCycle)
  {
    log::warning("BehaviourControl::setPlayMode") << "PlayMode set multiple times during think cycle, with values " << getPlayModeName(d_playMode) << " and " << getPlayModeName(playMode);
    ASSERT(false);
  }
  d_playMode = playMode;
  d_playModeSetThisCycle = true;
}
