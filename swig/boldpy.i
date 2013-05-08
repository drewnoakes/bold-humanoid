%module bold

// Includes inserted verbatim into wrapper code
%{
#include "../Agent/agent.hh"
#include "../OptionTree/optiontree.hh"
#include "../AgentState/agentstate.hh"
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
%}

// Can not create objects, unless we tell you to
%nodefaultctor;

// Include std library interfaces
%include <stl.i>
%include <std_shared_ptr.i>
%include "eigen.i"
%include "geometry.i"

// Have to list all classes of which a shared_ptr is used (plus their
// (grand)parent classes, just to be sure)
%shared_ptr(bold::StateObject)
%shared_ptr(bold::AgentFrameState)
%shared_ptr(bold::AlarmState)
%shared_ptr(bold::AmbulatorState)
%shared_ptr(bold::BodyState)
%shared_ptr(bold::CameraFrameState)
%shared_ptr(bold::DebugState)
%shared_ptr(bold::GameState)
%shared_ptr(bold::HardwareState)
%shared_ptr(bold::OptionTreeState)
%shared_ptr(bold::ParticleState)
%shared_ptr(bold::WorldFrameState)

%template() std::vector<PyObject*>;

// Now define all interfaces that we want to be available in Python In
// theory we can also %include all header files, but that often breaks
// (eg C++11 stuff, and usually inlined and private stuff)
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

  class StateObject
  {
  };

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

  class BodyState : public StateObject
  {
  public:
    BodyState(double angles[]);

    double getTorsoHeight() const;
  };

  class CameraFrameState : public StateObject
  {
  public:
    bool isBallVisible() const;
     std::vector<bold::LineSegment2i> getObservedLineSegments() const;
  };
  
  %extend CameraFrameState {
  public:
    // SWIG has issues with Maybe
    std::shared_ptr<Eigen::Vector2d> getBallObservation() const
    {
      return ($self->getBallObservation());
    }

    // 
    std::vector<PyObject*> getGoalObservations() const
    {
      std::vector<PyObject*> out;
      std::vector<Eigen::Vector2d> in = $self->getGoalObservations();
      for (int i = 0; i < in.size(); ++i)
      {
        PyObject *resultobj = 0;
        ConvertFromEigenToNumPyMatrix<Eigen::Vector2d>(&resultobj, &(in[i]));
        out.push_back(resultobj);
      }
      return out;
    }
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


