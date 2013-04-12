#include "hardwarestate.hh"

#include "../../CM730Snapshot/CM730Snapshot.hh"
#include "../../MX28Snapshot/MX28Snapshot.hh"

using namespace bold;
using namespace std;

void HardwareState::update(shared_ptr<CM730Snapshot> cm730State, vector<shared_ptr<MX28Snapshot>> const& mx28States)
{
  d_cm730State = cm730State;
  d_mx28States = mx28States;

  for (int jointId = Robot::JointData::ID_R_SHOULDER_PITCH; jointId < Robot::JointData::NUMBER_OF_JOINTS; jointId++)
  {
    auto mx28 = mx28States[jointId];

    // If the alarm state for an MX28 has changed, print it out
    // TODO do we need to examine mx28.alarmShutdown as well? it seems to hold the same flags as mx28.alarmLed
    if (mx28->alarmLed != d_alarmLedByJointId[jointId])
    {
      d_alarmLedByJointId[jointId] = mx28->alarmLed;
      cerr << "[BodyState::update] MX28[id=" << jointId << "] alarmLed flags changed: " << mx28->alarmLed << endl;
    }

//     if (mx28.alarmLed.hasError())
//       cerr << "[Agent::readSubBoardData] MX28[id=" << jointId << "] alarmLed flags: " << mx28.alarmLed << endl;

//     if (mx28.alarmShutdown.hasError())
//       cerr << "[Agent::readSubBoardData] MX28[id=" << jointId << "] alarmShutdown flags: " << mx28.alarmShutdown << endl;
  }
};
