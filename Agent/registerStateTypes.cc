#include "agent.ih"

void Agent::registerStateTypes()
{
  cout << "[Agent::registerStateTypes] Start" << endl;

  AgentState::getInstance().registerStateType<AgentFrameState>("AgentFrame");
  AgentState::getInstance().registerStateType<AmbulatorState>("Ambulator");
  AgentState::getInstance().registerStateType<BodyState>("Body");
  AgentState::getInstance().registerStateType<CameraFrameState>("CameraFrame");
  AgentState::getInstance().registerStateType<DebugState>("Debug");
  AgentState::getInstance().registerStateType<MotionTimingState>("MotionTiming");
  AgentState::getInstance().registerStateType<ThinkTimingState>("ThinkTiming");
  AgentState::getInstance().registerStateType<GameState>("Game");
  AgentState::getInstance().registerStateType<HardwareState>("Hardware");
  AgentState::getInstance().registerStateType<OptionTreeState>("OptionTree");
  AgentState::getInstance().registerStateType<ParticleState>("Particle");
  AgentState::getInstance().registerStateType<StaticHardwareState>("StaticHardware");
  AgentState::getInstance().registerStateType<WorldFrameState>("WorldFrame");
}
