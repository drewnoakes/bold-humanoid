#include "../StateObject/AgentFrameState/agentframestate.hh"
#include "../StateObject/AlarmState/alarmstate.hh"
#include "../StateObject/BodyState/bodystate.hh"
#include "../StateObject/CameraFrameState/cameraframestate.hh"
#include "../StateObject/GameState/gamestate.hh"
#include "../StateObject/HardwareState/hardwarestate.hh"
#include "../StateObject/WorldFrameState/worldframestate.hh"

#include "agentstate.hh"

using namespace bold;
using namespace std;

void AgentState::setAgentFrame(shared_ptr<AgentFrameState> agentFrame)
{
  d_agentFrame = agentFrame;
  updated(StateType::AgentFrame, dynamic_pointer_cast<StateObject>(d_agentFrame));
}

void AgentState::setAlarmState(shared_ptr<AlarmState> alarmState)
{
  d_alarmState = alarmState;
  updated(StateType::Alarm, dynamic_pointer_cast<StateObject>(d_alarmState));
}

void AgentState::setBodyState(shared_ptr<BodyState> bodyState)
{
  d_bodyState = bodyState;
  updated(StateType::Body, dynamic_pointer_cast<StateObject>(d_bodyState));
}

void AgentState::setCameraFrame(shared_ptr<CameraFrameState> cameraFrame)
{
  d_cameraFrameNumber++;
  d_cameraFrame = cameraFrame;
  updated(StateType::CameraFrame, dynamic_pointer_cast<StateObject>(d_cameraFrame));
}

void AgentState::setGameState(shared_ptr<GameState> gameState)
{
  d_gameState = gameState;
  updated(StateType::Game, dynamic_pointer_cast<StateObject>(d_gameState));
}

void AgentState::setHardwareState(shared_ptr<HardwareState> hardwareState)
{
  d_hardwareState = hardwareState;
  updated(StateType::Hardware, dynamic_pointer_cast<StateObject>(d_hardwareState));
}

void AgentState::setWorldFrame(shared_ptr<WorldFrameState> worldFrameState)
{
  d_worldFrameState = worldFrameState;
  updated(StateType::WorldFrame, dynamic_pointer_cast<StateObject>(d_worldFrameState));
}