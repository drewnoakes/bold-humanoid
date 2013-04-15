#include "agent.ih"

void Agent::registerStateTypes()
{
  cout << "[Agent::registerStateTypes] Start" << endl;

  AgentState::getInstance().registerStateType<AgentFrameState>("AgentFrame");
  AgentState::getInstance().registerStateType<AlarmState>("Alarm");
  AgentState::getInstance().registerStateType<BodyState>("Body");
  AgentState::getInstance().registerStateType<CameraFrameState>("CameraFrame");
  AgentState::getInstance().registerStateType<GameState>("Game");
  AgentState::getInstance().registerStateType<HardwareState>("Hardware");
  AgentState::getInstance().registerStateType<OptionTreeState>("OptionTree");
  AgentState::getInstance().registerStateType<WorldFrameState>("WorldFrame");
}
