#pragma once

#include <memory>

#include "../GameStateReceiver/gamecontrollertypes.hh"
#include "../StateObject/TeamState/teamstate.hh"

namespace bold
{
  class Agent;

  class BehaviourControl
  {
  public:
    BehaviourControl(Agent& agent);

    PlayerRole getPlayerRole() const { return d_playerRole; }
    PlayerStatus getPlayerStatus() const { return d_playerStatus; }
    PlayerActivity getPlayerActivity() const { return d_playerActivity; }
    robocup::PlayMode getPlayMode() const { return d_playMode; }

    void setPlayerRole(PlayerRole role);
    void setPlayerStatus(PlayerStatus status);
    void setPlayerActivity(PlayerActivity activity);
    void setPlayMode(robocup::PlayMode playMode);

    void updateStateObject();

  private:
    Agent& d_agent;
    robocup::PlayMode d_playMode;
    PlayerRole d_playerRole;
    PlayerActivity d_playerActivity;
    PlayerStatus d_playerStatus;

    bool d_playerRoleSetThisCycle;
    bool d_playerStatusSetThisCycle;
    bool d_playerActivitySetThisCycle;
    bool d_playModeSetThisCycle;
  };
}
