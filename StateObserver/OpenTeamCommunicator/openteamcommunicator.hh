#pragma once

#include <vector>

#include "../../Clock/clock.hh"
#include "../../StateObject/TeamState/teamstate.hh"
#include "../../StateObserver/stateobserver.hh"

namespace bold
{
  class BehaviourControl;
  class SequentialTimer;
  template<typename> class Setting;

  class OpenTeamCommunicator : public StateObserver
  {
  public:
    OpenTeamCommunicator(std::shared_ptr<BehaviourControl> behaviourControl);

    void observe(SequentialTimer& timer) override;

    void receiveData();

    void sendData(PlayerState& state);

  private:
    static PlayerRole     decodePlayerRole(int value);
    static PlayerActivity decodePlayerActivity(int value);
    static PlayerStatus   decodePlayerStatus(int value);

    static int encodePlayerRole(PlayerRole role);
    static int encodePlayerActivity(PlayerActivity activity);
    static int encodePlayerStatus(PlayerStatus status);

    void mergePlayerState(PlayerState& state);
    void updateStateObject();

    std::shared_ptr<BehaviourControl> d_behaviourControl;
    const uchar d_teamNumber;
    const uchar d_uniformNumber;
    const int d_localPort;
    const int d_remotePort;
    Setting<double>* d_sendPeriodSeconds;
    Setting<int>* d_maxPlayerDataAgeMillis;
    Clock::Timestamp d_lastBroadcast;
    int d_sock;
    std::vector<PlayerState> d_players;
  };
}
