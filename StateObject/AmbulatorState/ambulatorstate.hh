#ifndef BOLD_AMBULATORSTATE_HH
#define BOLD_AMBULATORSTATE_HH

#include "../stateobject.hh"

namespace bold
{
  class AmbulatorState : public StateObject
  {
  public:
    AmbulatorState(double targetX, double targetY, double targetTurn, double currentX, double currentY, double currentTurn)
    : d_targetX(targetX),
      d_targetY(targetY),
      d_targetTurn(targetTurn),
      d_currentX(currentX),
      d_currentY(currentY),
      d_currentTurn(currentTurn)
    {}

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    double d_targetX;
    double d_targetY;
    double d_targetTurn;
    double d_currentX;
    double d_currentY;
    double d_currentTurn;
  };
}

#endif
