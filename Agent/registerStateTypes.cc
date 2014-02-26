#include "agent.ih"

void Agent::registerStateTypes()
{
  AgentState::registerStateType<AgentFrameState>("AgentFrame");
  AgentState::registerStateType<AmbulatorState>("Ambulator");
  AgentState::registerStateType<BodyControlState>("BodyControl");
  AgentState::registerStateType<BodyState>("Body");
  AgentState::registerStateType<CameraFrameState>("CameraFrame");
  AgentState::registerStateType<DebugState>("Debug");
  AgentState::registerStateType<GameState>("Game");
  AgentState::registerStateType<HardwareState>("Hardware");
  AgentState::registerStateType<LabelCountState>("LabelCount");
  AgentState::registerStateType<MotionTaskState>("MotionTask");
  AgentState::registerStateType<MotionTimingState>("MotionTiming");
  AgentState::registerStateType<OdometryState>("Odometry");
  AgentState::registerStateType<OptionTreeState>("OptionTree");
  AgentState::registerStateType<OrientationState>("Orientation");
  AgentState::registerStateType<ParticleState>("Particle");
  AgentState::registerStateType<StaticHardwareState>("StaticHardware");
  AgentState::registerStateType<ThinkTimingState>("ThinkTiming");
  AgentState::registerStateType<WorldFrameState>("WorldFrame");
}
