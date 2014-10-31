#include "walkstate.hh"

#include "../../WalkEngine/walkengine.hh"
#include "../../MotionModule/WalkModule/walkmodule.hh"

using namespace bold;
using namespace rapidjson;
using namespace std;

WalkState::WalkState(
  double targetX, double targetY, double targetTurn, double targetHipPitch,
  double lastXDelta, double lastYDelta, double lastTurnDelta, double lastHipPitchDelta,
  WalkModule* walkModule,
  shared_ptr<WalkEngine> walkEngine)
: d_isRunning(walkModule->isRunning()),
  d_status(walkModule->getStatus()),
  d_targetX(targetX),
  d_targetY(targetY),
  d_targetTurn(targetTurn),
  d_targetHipPitch(targetHipPitch),
  d_currentX(walkEngine->X_MOVE_AMPLITUDE),
  d_currentY(walkEngine->Y_MOVE_AMPLITUDE),
  d_currentTurn(walkEngine->A_MOVE_AMPLITUDE),
  d_currentHipPitch(walkEngine->HIP_PITCH_OFFSET),
  d_lastXDelta(lastXDelta),
  d_lastYDelta(lastYDelta),
  d_lastTurnDelta(lastTurnDelta),
  d_lastHipPitchDelta(lastHipPitchDelta),
  d_currentPhase(walkEngine->getCurrentPhase()),
  d_bodySwingY(walkEngine->getBodySwingY()),
  d_bodySwingZ(walkEngine->getBodySwingZ())
{}
