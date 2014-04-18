%{
#include <StateObject/AmbulatorState/ambulatorstate.hh>
%}

namespace bold
{
  class AmbulatorState : public StateObject
  {
  public:
    double getTargetX() const;
    double getTargetY() const;
    double getTargetTurn() const;

    double getCurrentX() const;
    double getCurrentY() const;
    double getCurrentTurn() const;

    bool isRunning() const;

    int getCurrentPhase() const;

    double getBodySwingY() const;
    double getBodySwingZ() const;

    double getHipPitch() const;
  };
}
