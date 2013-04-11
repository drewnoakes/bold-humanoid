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
    void update(std::shared_ptr<CM730Snapshot> cm730State, std::shared_ptr<std::vector<MX28Snapshot>> mx28States)
    {
      d_cm730State = cm730State;
      d_mx28States = mx28States;

      // If the alarm state for an MX28 has changed, print it out
      for (int jointId = Robot::JointData::ID_R_SHOULDER_PITCH; jointId < Robot::JointData::NUMBER_OF_JOINTS; jointId++)
      {
        auto const& mx28 = (*mx28States)[jointId];

        // TODO do we need to examine mx28.alarmShutdown as well? it seems to hold the same flags as mx28.alarmLed
        if (mx28.alarmLed != d_alarmLedByJointId[jointId])
        {
          d_alarmLedByJointId[jointId] = mx28.alarmLed;
          std::cerr << "[BodyState::update] MX28[id=" << jointId << "] alarmLed flags changed: "
              << mx28.alarmLed << std::endl;
        }

//     if (mx28.alarmLed.hasError())
//     {
//       cerr << "[Agent::readSubBoardData] MX28[id=" << jointId << "] alarmLed flags: "
//            << mx28.alarmLed << endl;
//     }

//     if (mx28.alarmShutdown.hasError())
//     {
//       cerr << "[Agent::readSubBoardData] MX28[id=" << jointId << "] alarmShutdown flags: "
//            << mx28.alarmShutdown << endl;
//     }
      }
    };

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