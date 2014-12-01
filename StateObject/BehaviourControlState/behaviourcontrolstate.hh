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

    ulong getMotionCycleNumber() const { return d_motionCycleNumber; }
    PlayerRole getPlayerRole() const { return d_playerRole; }
    PlayerActivity getPlayerActivity() { return d_playerActivity; }
    PlayerStatus getPlayerStatus() const { return d_playerStatus; }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    ulong d_motionCycleNumber;
    PlayerRole d_playerRole;
    PlayerActivity d_playerActivity;
    PlayerStatus d_playerStatus;
  };

  template<typename TBuffer>
  inline void BehaviourControlState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
  {
    writer.StartObject();
    {
      writer.String("role");
      writer.Uint(static_cast<uint>(d_playerRole));
      writer.String("activity");
      writer.Uint(static_cast<uint>(d_playerActivity));
      writer.String("status");
      writer.Uint(static_cast<uint>(d_playerStatus));
    }
    writer.EndObject();
  }
}
