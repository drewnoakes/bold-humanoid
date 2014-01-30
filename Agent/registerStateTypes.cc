#include "agent.ih"

void Agent::registerStateTypes()
{
  AgentState::registerStateType<AgentFrameState>("AgentFrame");
  AgentState::registerStateType<AmbulatorState>("Ambulator");
  AgentState::registerStateType<BodyState>("Body");
  AgentState::registerStateType<BodyControlState>("BodyControl");
  AgentState::registerStateType<CameraFrameState>("CameraFrame");
  AgentState::registerStateType<DebugState>("Debug");
  AgentState::registerStateType<MotionTimingState>("MotionTiming");
  AgentState::registerStateType<LabelCountState>("LabelCount");
  AgentState::registerStateType<ThinkTimingState>("ThinkTiming");
  AgentState::registerStateType<GameState>("Game");
  AgentState::registerStateType<HardwareState>("Hardware");
  AgentState::registerStateType<MotionTaskState>("MotionTask");
  AgentState::registerStateType<OptionTreeState>("OptionTree");
  AgentState::registerStateType<OrientationState>("Orientation");
  AgentState::registerStateType<ParticleState>("Particle");
  AgentState::registerStateType<StaticHardwareState>("StaticHardware");
  AgentState::registerStateType<WorldFrameState>("WorldFrame");
}
