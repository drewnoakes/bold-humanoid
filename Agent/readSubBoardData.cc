#include "agent.ih"

#include "../StateObject/AlarmState/alarmstate.hh"

void Agent::readSubBoardData()
{
  //
  // READ HARDWARESTATE (input)
  //

  // TODO don't call this every step
  d_CM730->MakeBulkReadPacket();
  int res = d_CM730->BulkRead();

  if (res != CM730::SUCCESS)
  {
    // TODO set the 'Hardware' state as failing in AgentState, and broadcast error status to clients
    cout << "[Agent::readSubBoardData] Bulk read failed!" << endl;
    return;
  }

  auto cm730Snapshot = make_shared<CM730Snapshot>(d_CM730->m_BulkReadData[CM730::ID_CM]);

  auto mx28Snapshots = vector<shared_ptr<MX28Snapshot const>>();
  mx28Snapshots.push_back(make_shared<MX28Snapshot>()); // padding as joints start at 1
  for (unsigned jointId = 1; jointId < JointData::NUMBER_OF_JOINTS; jointId++)
  {
    auto mx28 = make_shared<MX28Snapshot>(d_CM730->m_BulkReadData[jointId], jointId);
    mx28Snapshots.push_back(mx28);
  }

  auto hw = make_shared<HardwareState const>(cm730Snapshot, mx28Snapshots);

  AgentState::getInstance().set(hw);

  //
  // UPDATE ALARMSTATE (trigger)
  //

  auto lastAlarmState = AgentState::get<AlarmState>();
  bool hasAlarmChanged = false;
  vector<MX28Alarm> alarmLedByJointId;
  alarmLedByJointId.push_back(MX28Alarm()); // offset, as jointIds start at 1
  for (unsigned jointId = 1; jointId < robotis::JointData::NUMBER_OF_JOINTS; jointId++)
  {
    // TODO do we need to examine mx28.alarmShutdown as well? it seems to hold the same flags as mx28.alarmLed
    auto alarmLed = hw->getMX28State(jointId)->alarmLed;

    alarmLedByJointId.push_back(alarmLed);

    // If the alarm state for an MX28 has changed, print it out
    if (!lastAlarmState || alarmLed != lastAlarmState->getAlarmLed(jointId))
    {
      hasAlarmChanged = true;
      cerr << "[Agent::readSubBoardData] MX28[id=" << jointId << "] alarmLed flags changed: " << alarmLed << endl;
    }
  }

  if (hasAlarmChanged || !lastAlarmState)
  {
    AgentState::getInstance().set(make_shared<AlarmState const>(alarmLedByJointId));
  }

  //
  // UPDATE BODYSTATE (trigger)
  //

  double angles[JointData::NUMBER_OF_JOINTS];
  for (unsigned i = 1; i < JointData::NUMBER_OF_JOINTS; i++)
  {
    angles[i] = hw->getMX28State(i)->presentPosition;
  }
  AgentState::getInstance().set(make_shared<BodyState const>(angles));
}
