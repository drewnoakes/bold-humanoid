#include "agent.ih"

void Agent::readStaticHardwareState()
{
  auto bulkRead = make_shared<BulkRead>(CM730::P_MODEL_NUMBER_L, CM730::P_RETURN_LEVEL,
                                        MX28::P_MODEL_NUMBER_L, MX28::P_TORQUE_ENABLE);

  int res = d_cm730->bulkRead(bulkRead);

  if (res != CM730::SUCCESS)
  {
    cerr << "[Agent::readStaticHardwareState] Bulk read failed -- skipping update of StaticHardwareState" << endl;
    return;
  }

  auto cm730State = make_shared<StaticCM730State>(bulkRead->getBulkReadData(CM730::ID_CM));

  auto mx28States = vector<shared_ptr<StaticMX28State const>>();
  mx28States.push_back(nullptr); // padding as joints start at 1
  for (unsigned jointId = 1; jointId <= 20; jointId++)
  {
    auto mx28 = make_shared<StaticMX28State>(bulkRead->getBulkReadData(jointId), jointId);
    mx28States.push_back(mx28);
  }

  AgentState::getInstance().set(make_shared<StaticHardwareState const>(cm730State, mx28States));
}
