#pragma once

#include "../../Clock/clock.hh"
#include "../../mitecom/mitecom-data.h"
#include "../../StateObserver/stateobserver.hh"

namespace bold
{
  template<typename> class Setting;

  class OpenTeamCommunicator : public StateObserver
  {
  public:
    OpenTeamCommunicator(unsigned teamNumber, unsigned uniformNumber);

    void observe(SequentialTimer& timer) override;

    void receiveData();

    void sendData();

  private:
    const unsigned d_teamNumber;
    const unsigned d_uniformNumber;
    const int d_localPort;
    const int d_remotePort;
    Setting<double>* d_sendPeriodSeconds;
    Clock::Timestamp d_lastBroadcast;
    int d_sock;
    MixedTeamMates d_teamMates;
  };
}
