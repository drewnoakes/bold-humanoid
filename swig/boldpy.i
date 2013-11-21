%module(directors="1") bold

// Can not create objects, unless we tell you to
%nodefaultctor;
// Create a lot of docs
%feature("autodoc", "3");

// C++ director exception handling
// Added to wrapping code
%feature("director:except") {
  if ($error != NULL) {
    PyErr_Print();
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
%shared_ptr(bold::Agent)
%shared_ptr(bold::AgentFrameState)
%shared_ptr(bold::AlarmState)
%shared_ptr(bold::Ambulator)
%shared_ptr(bold::AmbulatorState)
%shared_ptr(bold::BodyPart)
%shared_ptr(bold::BodyState)
%shared_ptr(bold::CameraFrameState)
%shared_ptr(bold::CameraModel)
%shared_ptr(bold::CM730Snapshot)
%shared_ptr(bold::Debugger)
%shared_ptr(bold::DebugState)
%shared_ptr(bold::GameState)
%shared_ptr(bold::HardwareState)
%shared_ptr(bold::HeadModule)
%shared_ptr(bold::Joint)
%shared_ptr(bold::Limb)
%shared_ptr(bold::MotionModule)
%shared_ptr(bold::MotionScriptRunner)
%shared_ptr(bold::MotionScriptModule)
%shared_ptr(bold::MX28Snapshot)
%shared_ptr(bold::Option)
%shared_ptr(bold::OptionTreeState)
%shared_ptr(bold::ParticleState)
%shared_ptr(bold::StateObject)
%shared_ptr(bold::StopWalking)
%shared_ptr(bold::VisualCortex)
%shared_ptr(bold::WalkModule)
%shared_ptr(bold::WorldFrameState)

// List special STL container instantiations
%template() std::vector<PyObject*>;
%template() std::vector<std::shared_ptr<bold::Option> >;

// Now define all interfaces that we want to be available in Python In
// theory we can also %include all header files, but that often breaks
// (eg C++11 stuff, and usually inlined and private stuff)
%feature("kwargs");

%include "../Debugger/debugger.i"
%include "../VisualCortex/visualcortex.i"
%include "../Agent/agent.i"
%include "../BodyPart/bodypart.i"
%include "../CameraModel/cameramodel.i"
%include "../CM730Snapshot/cm730snapshot.i"
%include "../MX28Snapshot/mx28snapshot.i"

%include "../StateObject/stateobject.i"
%include "../StateObject/AgentFrameState/agentframestate.i"
%include "../StateObject/AmbulatorState/ambulatorstate.i"
%include "../StateObject/BodyState/bodystate.i"
%include "../StateObject/CameraFrameState/cameraframestate.i"
%include "../StateObject/GameState/gamestate.i"
%include "../StateObject/HardwareState/hardwarestate.i"

%include "../AgentState/agentstate.i"

%include "../MotionModule/motionmodule.i"

%include "../Ambulator/ambulator.i"

%include "../Option/option.i"

%include "../OptionTree/optiontree.i"
