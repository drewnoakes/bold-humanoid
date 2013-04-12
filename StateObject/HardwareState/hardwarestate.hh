#ifndef BOLD_HARDWARESTATE_HH
#define BOLD_HARDWARESTATE_HH

#include <memory>
#include <vector>
#include <iostream>

#include "../stateobject.hh"
#include "../../robotis/Framework/include/JointData.h"
// TODO: header file name should be lower case
#include "../../MX28Alarm/MX28Alarm.hh"

namespace bold
{
  class CM730Snapshot;
  class MX28Snapshot;

  class HardwareState : public StateObject
  {
  public:
    HardwareState()
    {}

    void update(std::shared_ptr<CM730Snapshot> cm730State, std::vector<std::shared_ptr<MX28Snapshot>> const& mx28States);

    std::shared_ptr<CM730Snapshot const> getCM730State() const
    {
      return d_cm730State;
    }

    std::shared_ptr<MX28Snapshot const> getMX28State(unsigned jointId) const
    {
      assert(jointId > 0 && jointId < Robot::JointData::NUMBER_OF_JOINTS);

      return d_mx28States[jointId];
    }

  private:
    MX28Alarm d_alarmLedByJointId[Robot::JointData::NUMBER_OF_JOINTS];
    std::shared_ptr<CM730Snapshot> d_cm730State;
    std::vector<std::shared_ptr<MX28Snapshot>> d_mx28States;
  };
}

#endif
