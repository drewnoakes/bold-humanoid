#pragma once

#include "../../MotionModule/WalkModule/walkmodule.hh"
#include "../stateobject.hh"

namespace bold
{
  class AmbulatorState : public StateObject
  {
  public:
    AmbulatorState(double targetX, double targetY, double targetTurn, std::shared_ptr<WalkModule> walker)
    : d_targetX(targetX),
      d_targetY(targetY),
      d_targetTurn(targetTurn),
      d_currentX(walker->X_MOVE_AMPLITUDE),
      d_currentY(walker->Y_MOVE_AMPLITUDE),
      d_currentTurn(walker->A_MOVE_AMPLITUDE),
      d_isRunning(walker->isRunning()),
      d_currentPhase(walker->getCurrentPhase()),
      d_bodySwingY(walker->getBodySwingY()),
      d_bodySwingZ(walker->getBodySwingZ()),
      d_hipPitch(walker->HIP_PITCH_OFFSET)
    {}

    double getTargetX() const { return d_targetX; }
    double getTargetY() const { return d_targetY; }
    double getTargetTurn() const { return d_targetTurn; }

    double getCurrentX() const { return d_currentX; }
    double getCurrentY() const { return d_currentY; }
    double getCurrentTurn() const { return d_currentTurn; }

    bool isRunning() const { return d_isRunning; }

    int getCurrentPhase() const { return d_currentPhase; }

    double getBodySwingY() const { return d_bodySwingY; }
    double getBodySwingZ() const { return d_bodySwingZ; }

    double getHipPitch() const { return d_hipPitch; }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    double d_targetX;
    double d_targetY;
    double d_targetTurn;
    double d_currentX;
    double d_currentY;
    double d_currentTurn;
    bool d_isRunning;
    int d_currentPhase;
    double d_bodySwingY;
    double d_bodySwingZ;
    double d_hipPitch;
  };
}
