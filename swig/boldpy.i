%module(directors="1") bold

// Can not create objects, unless we tell you to
%nodefaultctor;
// Create a lot of docs
%feature("autodoc", "3");

%feature("director:except") {
    if ($error != NULL) {
        throw Swig::DirectorMethodException();
    }
}

// Include std library interfaces
%include <stl.i>
%include <std_shared_ptr.i>
%include "eigen.i"
%include "geometry.i"

// Have to list all classes of which a shared_ptr is used (plus their
// (grand)parent classes, just to be sure). Must be listed before any
// use
%shared_ptr(bold::Configurable)
%shared_ptr(bold::Agent)
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

%shared_ptr(bold::VisualCortex)

%shared_ptr(bold::BodyPart)
%shared_ptr(bold::Limb)
%shared_ptr(bold::Joint)

%shared_ptr(bold::Option)
%shared_ptr(bold::ActionOption)
%shared_ptr(bold::FSMState)
%shared_ptr(bold::FSMTransition)
%shared_ptr(bold::FSMOption)

%template() std::vector<PyObject*>;
%template() std::vector<std::shared_ptr<bold::Option> >;

// Now define all interfaces that we want to be available in Python In
// theory we can also %include all header files, but that often breaks
// (eg C++11 stuff, and usually inlined and private stuff)
%feature("kwargs");

%include "../Configurable/configurable.i"
%include "../VisualCortex/visualcortex.i"
%include "../Agent/agent.i"
%include "../BodyPart/bodypart.i"
%include "../StateObject/stateobject.i"
%include "../StateObject/BodyState/bodystate.i"
%include "../AgentState/agentstate.i"

%include "../Option/option.i"
%include "../Option/ActionOption/actionoption.i"
%include "../Option/FSMOption/fsmoption.i"

%include "../OptionTree/optiontree.i"

namespace bold
{

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
