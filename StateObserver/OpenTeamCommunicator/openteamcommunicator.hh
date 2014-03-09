#pragma once

#include "../../Clock/clock.hh"
#include "../../mitecom/mitecom-network.h"
#include "../../mitecom/mitecom-handler.h"
#include "../../StateObject/AgentFrameState/agentframestate.hh"
#include "../../StateObject/WorldFrameState/worldframestate.hh"
#include "../../StateObserver/stateobserver.hh"
#include "../../util/Maybe.hh"

#include <Eigen/Core>
#include <memory>

namespace bold
{
  /** State of communication
   */
  enum class OpenTeamCommunicatorStatus
  {
    IDLE,
    WAITING,
    SENDING,
    RECEIVING,
    SOCKET_FAILURE
  };

  class OpenTeamCommunicator : public StateObserver
  {
  public:
    OpenTeamCommunicator(const unsigned teamNumber, const unsigned uniformNumber);

    void observe(SequentialTimer& timer) override;

    OpenTeamCommunicatorStatus receiveData();

    void update(MixedTeamMate const& mate);

    OpenTeamCommunicatorStatus sendData();

    OpenTeamCommunicatorStatus getCommunicatorState() const { return d_commState; }

  private:
    OpenTeamCommunicatorStatus d_commState;
    const unsigned d_teamNumber;
    const unsigned d_uniformNumber;
    Clock::Timestamp d_currentTime;
    Clock::Timestamp d_lastBroadcast;
    signed d_sock;
    MixedTeamMates d_teamMates;
  };
}
