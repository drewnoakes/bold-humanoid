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

void WalkState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("running");
    writer.Bool(d_isRunning);
    writer.String("status");
    writer.Int((int)d_status);

    writer.String("target");
    writer.StartArray();
    {
      writer.Double(d_targetX, "%.2f");
      writer.Double(d_targetY, "%.2f");
      writer.Double(d_targetTurn, "%.2f");
    }
    writer.EndArray();

    writer.String("current");
    writer.StartArray();
    {
      writer.Double(d_currentX, "%.2f");
      writer.Double(d_currentY, "%.2f");
      writer.Double(d_currentTurn, "%.2f");
      writer.Double(d_currentHipPitch, "%.2f");
    }
    writer.EndArray();

    writer.String("delta");
    writer.StartArray();
    {
      writer.Double(d_lastXDelta, "%.2f");
      writer.Double(d_lastYDelta, "%.2f");
      writer.Double(d_lastTurnDelta, "%.2f");
      writer.Double(d_lastHipPitchDelta, "%.2f");
    }
    writer.EndArray();

    writer.String("phase");
    writer.Int(d_currentPhase);

    writer.String("hipPitch");
    writer.StartObject();
    writer.String("target");
    writer.Double(d_targetHipPitch, "%.3f");
    writer.String("current");
    writer.Double(d_currentHipPitch, "%.3f");
    writer.String("delta");
    writer.Double(d_lastHipPitchDelta, "%.3f");
    writer.EndObject();

    writer.String("bodySwingY");
    writer.Double(d_bodySwingY, "%.3f");
    writer.String("bodySwingZ");
    writer.Double(d_bodySwingZ, "%.3f");
  }
  writer.EndObject();
}
