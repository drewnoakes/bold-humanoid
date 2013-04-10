#include "agent.ih"

#include "../AgentModel/agentmodel.hh"
#include "../StateObject/BodyState/bodystate.hh"

void Agent::readSubBoardData()
{
  //
  // READ ALL DATA IN BULK
  //
  d_CM730.MakeBulkReadPacket();
  int res = d_CM730.BulkRead();

  if (res != CM730::SUCCESS)
  {
    cout << "[Agent::readSubBoardData] Bulk read failed!" << endl;
    return;
  }

  //
  // READ FROM SUB BOARD
  //
  CM730Snapshot cm730Snapshot;
  cm730Snapshot.init(d_CM730.m_BulkReadData[CM730::ID_CM]);

  //
  // READ FROM EACH JOINT
  //
  auto mx28Snapshots = make_shared<vector<MX28Snapshot>>();
  for (int jointId = JointData::ID_R_SHOULDER_PITCH; jointId < JointData::NUMBER_OF_JOINTS; jointId++)
  {
    MX28Snapshot mx28;
    mx28.init(d_CM730.m_BulkReadData[jointId], jointId);
    mx28Snapshots->push_back(mx28);
  }

  AgentState::getInstance().body()->update(make_shared<CM730Snapshot>(cm730Snapshot), mx28Snapshots);
}
