#include "agent.ih"
#include "../AgentModel/agentmodel.hh"

void Agent::readSubBoardData()
{
  auto& am = AgentModel::getInstance();


  //
  // READ ALL DATA IN BULK
  //
  d_CM730.BulkRead();

  //
  // READ FROM SUB BOARD
  //
  am.cm730State.init(d_CM730.m_BulkReadData[CM730::ID_CM]);

  //
  // READ FROM EACH JOINT
  //
  for (int jointId = JointData::ID_R_SHOULDER_PITCH; jointId < JointData::NUMBER_OF_JOINTS; jointId++)
  {
    MX28Snapshot& mx28 = am.mx28States[jointId];

    mx28.init(d_CM730.m_BulkReadData[jointId], jointId);

    // If the alarm state for an MX28 has changed, print it out
    // TODO do we need to examine mx28.alarmShutdown as well? it seems to hold the same flags as mx28.alarmLed
    if (mx28.alarmLed != d_alarmLedByJointId[jointId])
    {
      d_alarmLedByJointId[jointId] = mx28.alarmLed;
      cerr << "[Agent::readSubBoardData] MX28[id=" << jointId << "] alarmLed flags changed: "
           << mx28.alarmLed << endl;
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

  am.updated();
}
