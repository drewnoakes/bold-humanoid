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
    am.mx28States[jointId].init(d_CM730, jointId);
  }

  am.updated();
}