#pragma once

#include <memory>

#include "../stateobject.hh"
#include "../TeamState/teamstate.hh"

namespace bold
{
  typedef unsigned long ulong;

  class BehaviourControl;

  class BehaviourControlState : public StateObject
  {
  public:
    BehaviourControlState(ulong motionCycleNumber, PlayerRole playerRole, PlayerActivity playerActivity, PlayerStatus playerStatus)
    : d_motionCycleNumber(motionCycleNumber),
      d_playerRole(playerRole),
      d_playerActivity(playerActivity),
      d_playerStatus(playerStatus)
    {}

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    ulong d_motionCycleNumber;
    PlayerRole d_playerRole;
    PlayerActivity d_playerActivity;
    PlayerStatus d_playerStatus;
  };
}
