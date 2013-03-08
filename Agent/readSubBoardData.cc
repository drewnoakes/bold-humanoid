#include "agent.ih"
#include "../AgentModel/agentmodel.hh"

void Agent::readSubBoardData()
{
  auto& am = AgentModel::getInstance();

  //
  // READ FROM SUB BOARD
  //
  am.cm730State.init(d_CM730);

  //
  // READ FROM EACH JOINT
  //
  for (int jointId = JointData::ID_R_SHOULDER_PITCH; jointId < JointData::NUMBER_OF_JOINTS; jointId++)
  {
    MX28Snapshot& mx28 = am.mx28States[jointId];

    mx28.init(d_CM730, jointId);

    if (mx28.alarmLed.hasError())
    {
      cerr << "[Agent::readSubBoardData] MX28[id=" << jointId << "] alarmLed flags: "
           << mx28.alarmLed << endl;
    }

    if (mx28.alarmShutdown.hasError())
    {
      cerr << "[Agent::readSubBoardData] MX28[id=" << jointId << "] alarmShutdown flags: "
           << mx28.alarmShutdown << endl;
    }
  }

  am.updated();
}
