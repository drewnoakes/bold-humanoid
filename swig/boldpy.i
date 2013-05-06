%module bold
%{
#include "../Agent/agent.hh"
#include "../OptionTree/optiontree.hh"
#include "../AgentState/agentstate.hh"
#include "../StateObject/BodyState/bodystate.hh"
#include "../StateObject/AgentFrameState/agentframestate.hh"
#include "../StateObject/HardwareState/hardwarestate.hh"
#include "../StateObject/CameraFrameState/cameraframestate.hh"
#include "../StateObject/GameState/gamestate.hh"
#include "../StateObject/AmbulatorState/ambulatorstate.hh"
#include "../StateObject/ParticleState/particlestate.hh"
#include "../StateObject/WorldFrameState/worldframestate.hh"
#include "../StateObject/OptionTreeState/optiontreestate.hh"
#include "../StateObject/AlarmState/alarmstate.hh"
#include "../StateObject/BodyState/bodystate.hh"
#include "../StateObject/DebugState/debugstate.hh"

#include <functional>

%}

%include "std_string.i"

namespace bold
{
  class Agent
  {
  public:
    Agent(std::string const& U2D_dev,
          std::string const& confFile,
          std::string const& motionFile,
          unsigned teamNumber,
          unsigned uniformNumber,
          bool useJoystick,
          bool autoGetUpFromFallen,
          bool useOptionTree,
          bool recordFrames,
          bool ignoreGameController);

    void run();
    void stop();

  };

  %extend Agent {
  public:
    void onThinkEndConnect(PyObject* pyFunc)
    {
      $self->onThinkEnd.connect(
        [pyFunc]() {
          PyEval_CallObject(pyFunc, Py_BuildValue("()"));
        });
    }
  };

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

%template(getBodyState) bold::AgentState::get<bold::BodyState>;
%template(getAgentFrameState) bold::AgentState::get<bold::AgentFrameState>;
%template(getAlarmState) bold::AgentState::get<bold::AlarmState>;
%template(getAmbulatorState) bold::AgentState::get<bold::AmbulatorState>;
%template(getBodyState) bold::AgentState::get<bold::BodyState>;
%template(getCameraFrameState) bold::AgentState::get<bold::CameraFrameState>;
%template(getDebugState) bold::AgentState::get<bold::DebugState>;
%template(getGameState) bold::AgentState::get<bold::GameState>;
%template(getHardwareState) bold::AgentState::get<bold::HardwareState>;
%template(getOptionTreeState) bold::AgentState::get<bold::OptionTreeState>;
%template(getParticleState) bold::AgentState::get<bold::ParticleState>;
%template(getWorldFrameState) bold::AgentState::get<bold::WorldFrameState>;

