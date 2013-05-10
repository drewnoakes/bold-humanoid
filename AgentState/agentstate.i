%{
#include <AgentState/agentstate.hh>
%}

namespace bold
{
  class StateTracker;
  
  class AgentState
  {
  public:
    AgentState()
    {}
    
    sigc::signal<void, std::shared_ptr<StateTracker> > updated;
    
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
STATEOBJECT_TEMPLATE(AlarmState);
STATEOBJECT_TEMPLATE(AmbulatorState);
STATEOBJECT_TEMPLATE(BodyState);
STATEOBJECT_TEMPLATE(CameraFrameState);
STATEOBJECT_TEMPLATE(DebugState);
STATEOBJECT_TEMPLATE(GameState);
STATEOBJECT_TEMPLATE(HardwareState);
STATEOBJECT_TEMPLATE(OptionTreeState);
STATEOBJECT_TEMPLATE(ParticleState);
STATEOBJECT_TEMPLATE(WorldFrameState);


