#pragma once

#include "../stateobject.hh"
#include "../../MotionModule/WalkModule/walkmodule.hh"

namespace bold
{
  class WalkEngine;

  class WalkState : public StateObject
  {
  public:
    WalkState(double targetX, double targetY, double targetTurn,
                   double lastXDelta, double lastYDelta, double lastTurnDelta,
                   WalkModule* walkModule,
                   std::shared_ptr<WalkEngine> walkEngine);

    // From walk module

    bool isRunning() const { return d_isRunning; }
    WalkStatus getStatus() const { return d_status; }

    // From walk engine

    double getTargetX() const { return d_targetX; }
    double getTargetY() const { return d_targetY; }
    double getTargetTurn() const { return d_targetTurn; }

    double getCurrentX() const { return d_currentX; }
    double getCurrentY() const { return d_currentY; }
    double getCurrentTurn() const { return d_currentTurn; }

    int getCurrentPhase() const { return d_currentPhase; }

    double getBodySwingY() const { return d_bodySwingY; }
    double getBodySwingZ() const { return d_bodySwingZ; }

    double getHipPitch() const { return d_hipPitch; }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    // From walk module
    bool d_isRunning;
    WalkStatus d_status;

    // From walk engine
    double d_targetX;
    double d_targetY;
    double d_targetTurn;
    double d_currentX;
    double d_currentY;
    double d_currentTurn;
    double d_lastXDelta;
    double d_lastYDelta;
    double d_lastTurnDelta;
    int d_currentPhase;
    double d_bodySwingY;
    double d_bodySwingZ;
    double d_hipPitch;
  };
}
