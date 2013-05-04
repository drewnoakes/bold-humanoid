#pragma once

#include "../stateobject.hh"

#include <Walking.h>

namespace bold
{
  class AmbulatorState : public StateObject
  {
  public:
    AmbulatorState(double targetX, double targetY, double targetTurn, robotis::Walking* walker)
    : d_targetX(targetX),
      d_targetY(targetY),
      d_targetTurn(targetTurn),
      d_currentX(walker->X_MOVE_AMPLITUDE),
      d_currentY(walker->Y_MOVE_AMPLITUDE),
      d_currentTurn(walker->A_MOVE_AMPLITUDE),
      d_isRunning(walker->IsRunning()),
      d_currentPhase(walker->GetCurrentPhase()),
      d_bodySwingY(walker->GetBodySwingY()),
      d_bodySwingZ(walker->GetBodySwingZ())
    {
      robotis::JointData jd = walker->m_Joint;
      for (unsigned j = 1; j < robotis::JointData::NUMBER_OF_JOINTS; j++)
        d_enabled[j] = jd.GetEnable(j);
    }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    bool d_enabled[robotis::JointData::NUMBER_OF_JOINTS];
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
  };
}
