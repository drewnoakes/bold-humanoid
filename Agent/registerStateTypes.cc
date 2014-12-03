#include "agent.hh"

#include "../StateObject/AgentFrameState/agentframestate.hh"
#include "../StateObject/AudioPowerSpectrumState/audiopowerspectrumstate.hh"
#include "../StateObject/BalanceState/balancestate.hh"
#include "../StateObject/BehaviourControlState/behaviourcontrolstate.hh"
#include "../StateObject/BodyControlState/bodycontrolstate.hh"
#include "../StateObject/BodyState/bodystate.hh"
#include "../StateObject/CameraFrameState/cameraframestate.hh"
#include "../StateObject/DrawingState/drawingstate.hh"
#include "../StateObject/GameState/gamestate.hh"
#include "../StateObject/HardwareState/hardwarestate.hh"
#include "../StateObject/LabelCountState/labelcountstate.hh"
#include "../StateObject/LabelTeacherState/labelteacherstate.hh"
#include "../StateObject/LEDState/ledstate.hh"
#include "../StateObject/MessageCountState/messagecountstate.hh"
#include "../StateObject/MotionTaskState/motiontaskstate.hh"
#include "../StateObject/OdometryState/odometrystate.hh"
#include "../StateObject/OrientationState/orientationstate.hh"
#include "../StateObject/OptionTreeState/optiontreestate.hh"
#include "../StateObject/ParticleState/particlestate.hh"
#include "../StateObject/StaticHardwareState/statichardwarestate.hh"
#include "../StateObject/StationaryMapState/stationarymapstate.hh"
#include "../StateObject/TimingState/timingstate.hh"
#include "../StateObject/WalkState/walkstate.hh"
#include "../StateObject/WorldFrameState/worldframestate.hh"

#include "../State/state.hh"

using namespace bold;

void Agent::registerStateTypes()
{
  State::registerStateType<AgentFrameState>("AgentFrame");
  State::registerStateType<AudioPowerSpectrumState>("AudioPowerSpectrum");
  State::registerStateType<BalanceState>("Balance");
  State::registerStateType<BehaviourControlState>("BehaviourControl");
  State::registerStateType<BodyControlState>("BodyControl");
  State::registerStateType<BodyState>("Body");
  State::registerStateType<CameraFrameState>("CameraFrame");
  State::registerStateType<DrawingState>("Drawing");
  State::registerStateType<GameState>("Game");
  State::registerStateType<HardwareState>("Hardware");
  State::registerStateType<LabelCountState>("LabelCount");
  State::registerStateType<LabelTeacherState>("LabelTeacher");
  State::registerStateType<LEDState>("LED");
  State::registerStateType<MessageCountState>("MessageCount");
  State::registerStateType<MotionTaskState>("MotionTask");
  State::registerStateType<MotionTimingState>("MotionTiming");
  State::registerStateType<OdometryState>("Odometry");
  State::registerStateType<TeamState>("Team");
  State::registerStateType<OptionTreeState>("OptionTree");
  State::registerStateType<OrientationState>("Orientation");
  State::registerStateType<ParticleState>("Particle");
  State::registerStateType<StationaryMapState>("StationaryMap");
  State::registerStateType<StaticHardwareState>("StaticHardware");
  State::registerStateType<ThinkTimingState>("ThinkTiming");
  State::registerStateType<WalkState>("Walk");
  State::registerStateType<WorldFrameState>("WorldFrame");
}
