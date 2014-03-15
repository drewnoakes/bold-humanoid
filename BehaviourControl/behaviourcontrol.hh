#pragma once

#include <memory>

#include "../StateObject/TeamState/teamstate.hh"

namespace bold
{
  class Agent;

  class BehaviourControl
  {
  public:
    BehaviourControl(Agent& agent);

    PlayerRole getPlayerRole() const { return d_playerRole; }
    void setPlayerRole(PlayerRole role) { d_playerRole = role; }

    PlayerStatus getPlayerStatus() const { return d_playerStatus; }
    void setPlayerStatus(PlayerStatus status) { d_playerStatus = status; }

    PlayerActivity getPlayerActivity() const { return d_playerActivity; }
    void setPlayerActivity(PlayerActivity activity) { d_playerActivity = activity; }

  private:
    void updateStateObject() const;

    Agent& d_agent;
    PlayerRole d_playerRole;
    PlayerActivity d_playerActivity;
    PlayerStatus d_playerStatus;
  };
}
