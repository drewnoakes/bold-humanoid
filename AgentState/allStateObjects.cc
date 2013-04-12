#include "../StateObject/AgentFrameState/agentframestate.hh"
#include "../StateObject/AlarmState/alarmstate.hh"
#include "../StateObject/BodyState/bodystate.hh"
#include "../StateObject/CameraFrameState/cameraframestate.hh"
#include "../StateObject/GameState/gamestate.hh"
#include "../StateObject/HardwareState/hardwarestate.hh"

#include "agentstate.hh"

using namespace bold;
using namespace std;

vector<shared_ptr<StateObject>> AgentState::allStateObjects() const
{
  return {
    dynamic_pointer_cast<StateObject>(d_cameraFrame),
    dynamic_pointer_cast<StateObject>(d_agentFrame),
    dynamic_pointer_cast<StateObject>(d_gameState),
    dynamic_pointer_cast<StateObject>(d_hardwareState),
    dynamic_pointer_cast<StateObject>(d_bodyState),
    dynamic_pointer_cast<StateObject>(d_alarmState)
  };
}