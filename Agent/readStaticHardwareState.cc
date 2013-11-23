#include "agent.ih"

void Agent::readStaticHardwareState()
{
  auto staticBulkRead = make_shared<BulkRead>(CM730::P_MODEL_NUMBER_L, CM730::P_RETURN_LEVEL,
                                              MX28::P_MODEL_NUMBER_L, MX28::P_LOCK);

  CommResult res = d_cm730->bulkRead(staticBulkRead);

  if (res != CommResult::SUCCESS)
  {
    cerr << ccolor::warning << "[Agent::readStaticHardwareState] Bulk read failed -- skipping update of StaticHardwareState" << ccolor::reset << endl;
    return;
  }

  auto cm730State = make_shared<StaticCM730State>(staticBulkRead->getBulkReadData(CM730::ID_CM));

  auto mx28States = vector<shared_ptr<StaticMX28State const>>();
  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    mx28States.push_back(make_shared<StaticMX28State>(staticBulkRead->getBulkReadData(jointId), jointId));

  AgentState::getInstance().set(make_shared<StaticHardwareState const>(cm730State, mx28States));
}
