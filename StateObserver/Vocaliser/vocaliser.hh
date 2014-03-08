#pragma once

#include "../typedstateobserver.hh"
#include "../../StateObject/AgentFrameState/agentframestate.hh"

namespace bold
{
  template<typename> class Setting;
  class Voice;

  class Vocaliser : public TypedStateObserver<AgentFrameState>
  {
  public:
    Vocaliser(std::shared_ptr<Voice> voice);

    void observeTyped(std::shared_ptr<AgentFrameState const> const& agentFrameState, SequentialTimer& timer) override;

  private:
    Setting<bool>* d_enableBallPos;
    std::shared_ptr<Voice> d_voice;
  };
}
