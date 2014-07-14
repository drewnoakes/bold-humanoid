#pragma once

#include "../stateobject.hh"
#include "../../MotionModule/WalkModule/walkmodule.hh"

namespace bold
{
  class WalkEngine;

  class WalkState : public StateObject
  {
  public:
    WalkState(double targetX, double targetY, double targetTurn, double targetHipPitch,
              double lastXDelta, double lastYDelta, double lastTurnDelta, double lastHipPitchDelta,
              WalkModule* walkModule,
              std::shared_ptr<WalkEngine> walkEngine);

    // From walk module

    bool isRunning() const { return d_isRunning; }
    WalkStatus getStatus() const { return d_status; }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    // From walk module
    bool d_isRunning;
    WalkStatus d_status;

    // From walk engine
    double d_targetX;
    double d_targetY;
    double d_targetTurn;
    double d_targetHipPitch;
    double d_currentX;
    double d_currentY;
    double d_currentTurn;
    double d_currentHipPitch;
    double d_lastXDelta;
    double d_lastYDelta;
    double d_lastTurnDelta;
    double d_lastHipPitchDelta;
    int d_currentPhase;
    double d_bodySwingY;
    double d_bodySwingZ;
  };
}
