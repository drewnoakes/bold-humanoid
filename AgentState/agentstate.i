%{
#include <AgentState/agentstate.hh>
#include <StateObject/AgentFrameState/agentframestate.hh>
#include <StateObject/HardwareState/hardwarestate.hh>
#include <StateObject/CameraFrameState/cameraframestate.hh>
#include <StateObject/GameState/gamestate.hh>
#include <StateObject/AmbulatorState/ambulatorstate.hh>
#include <StateObject/ParticleState/particlestate.hh>
#include <StateObject/WorldFrameState/worldframestate.hh>
#include <StateObject/OptionTreeState/optiontreestate.hh>
#include <StateObject/BodyState/bodystate.hh>
#include <StateObject/DebugState/debugstate.hh>
%}

namespace bold
{
  class StateTracker;
  
  class AgentState
  {
  public:
    AgentState()
    {}
    
    template <typename T>
      static std::shared_ptr<T const> get();
    
    static AgentState& getInstance();
  };
}

// Must list all template instantiations
%define STATEOBJECT_TEMPLATE(O)
%template(get ## O) bold::AgentState::get<bold::O>;
%enddef

STATEOBJECT_TEMPLATE(AgentFrameState);
STATEOBJECT_TEMPLATE(AmbulatorState);
STATEOBJECT_TEMPLATE(BodyState);
STATEOBJECT_TEMPLATE(CameraFrameState);
STATEOBJECT_TEMPLATE(DebugState);
STATEOBJECT_TEMPLATE(GameState);
STATEOBJECT_TEMPLATE(HardwareState);
STATEOBJECT_TEMPLATE(OptionTreeState);
STATEOBJECT_TEMPLATE(ParticleState);
STATEOBJECT_TEMPLATE(WorldFrameState);


