#include "hardwarestate.hh"

#include "../../CM730Snapshot/CM730Snapshot.hh"
#include "../../MX28Snapshot/MX28Snapshot.hh"

using namespace bold;
using namespace std;

void HardwareState::update(shared_ptr<CM730Snapshot> cm730State, vector<shared_ptr<MX28Snapshot>> const& mx28States)
{
  assert(cm730State);

  cout << "copying states" << endl;
  d_cm730State = cm730State;
  cout << mx28States.size() << " " << d_mx28States.size() << endl;

  d_mx28States = mx28States;

  // If the alarm state for an MX28 has changed, print it out
  for (int jointId = Robot::JointData::ID_R_SHOULDER_PITCH; jointId < Robot::JointData::NUMBER_OF_JOINTS; jointId++)
  {
    cout << "joint: " << jointId << endl;
    assert(mx28States[jointId]);
    auto mx28 = mx28States[jointId];

    // TODO do we need to examine mx28.alarmShutdown as well? it seems to hold the same flags as mx28.alarmLed
    if (mx28->alarmLed != d_alarmLedByJointId[jointId])
    {
      d_alarmLedByJointId[jointId] = mx28->alarmLed;
      cerr << "[BodyState::update] MX28[id=" << jointId << "] alarmLed flags changed: "
          << mx28->alarmLed << endl;
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
