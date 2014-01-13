#include "agent.ih"

void Agent::registerStateTypes()
{
  AgentState::getInstance().registerStateType<AgentFrameState>("AgentFrame");
  AgentState::getInstance().registerStateType<AmbulatorState>("Ambulator");
  AgentState::getInstance().registerStateType<BodyState>("Body");
  AgentState::getInstance().registerStateType<BodyControlState>("BodyControl");
  AgentState::getInstance().registerStateType<CameraFrameState>("CameraFrame");
  AgentState::getInstance().registerStateType<DebugState>("Debug");
  AgentState::getInstance().registerStateType<MotionTimingState>("MotionTiming");
  AgentState::getInstance().registerStateType<LabelCountState>("LabelCount");
  AgentState::getInstance().registerStateType<ThinkTimingState>("ThinkTiming");
  AgentState::getInstance().registerStateType<GameState>("Game");
  AgentState::getInstance().registerStateType<HardwareState>("Hardware");
  AgentState::getInstance().registerStateType<MotionTaskState>("MotionTask");
  AgentState::getInstance().registerStateType<OptionTreeState>("OptionTree");
  AgentState::getInstance().registerStateType<OrientationState>("Orientation");
  AgentState::getInstance().registerStateType<ParticleState>("Particle");
  AgentState::getInstance().registerStateType<StaticHardwareState>("StaticHardware");
  AgentState::getInstance().registerStateType<WorldFrameState>("WorldFrame");
}
