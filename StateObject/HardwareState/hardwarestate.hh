#ifndef BOLD_HARDWARESTATE_HH
#define BOLD_HARDWARESTATE_HH

#include <memory>
#include <vector>
#include <iostream>

#include "../stateobject.hh"
#include "../../CM730Snapshot/CM730Snapshot.hh"
#include "../../MX28Snapshot/MX28Snapshot.hh"
#include "../../robotis/Framework/include/JointData.h"

namespace bold
{
  class HardwareState : public StateObject
  {
  public:
    void update(std::shared_ptr<CM730Snapshot> cm730State, std::shared_ptr<std::vector<MX28Snapshot>> mx28States);

    const CM730Snapshot& getCM730State() const
    {
      return *d_cm730State;
    }

    const MX28Snapshot& getMX28State(unsigned jointId) const
    {
      assert(jointId > 0 && jointId < Robot::JointData::NUMBER_OF_JOINTS);

      return (*d_mx28States)[jointId];
    }

  private:
    MX28Alarm d_alarmLedByJointId[Robot::JointData::NUMBER_OF_JOINTS];
    std::shared_ptr<CM730Snapshot> d_cm730State;
    std::shared_ptr<std::vector<MX28Snapshot>> d_mx28States;
  };
}

#endif