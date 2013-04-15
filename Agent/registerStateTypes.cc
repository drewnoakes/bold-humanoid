#include "agent.ih"

#include "../StateObject/AgentFrameState/agentframestate.hh"
#include "../StateObject/AlarmState/alarmstate.hh"
#include "../StateObject/BodyState/bodystate.hh"
#include "../StateObject/CameraFrameState/cameraframestate.hh"
#include "../StateObject/GameState/gamestate.hh"
#include "../StateObject/HardwareState/hardwarestate.hh"
#include "../StateObject/WorldFrameState/worldframestate.hh"

void Agent::registerStateTypes()
{
  cout << "[Agent::registerStateTypes] Start" << endl;

  AgentState::getInstance().registerStateType<AgentFrameState>("AgentFrame");
  AgentState::getInstance().registerStateType<AlarmState>("Alarm");
  AgentState::getInstance().registerStateType<BodyState>("Body");
  AgentState::getInstance().registerStateType<CameraFrameState>("CameraFrame");
  AgentState::getInstance().registerStateType<GameState>("Game");
  AgentState::getInstance().registerStateType<HardwareState>("Hardware");
  AgentState::getInstance().registerStateType<WorldFrameState>("WorldFrame");
}
