#include "jointid.hh"

using namespace bold;

bool bold::isHeadJoint(JointId id)
{
  return id == JointId::HEAD_PAN || id == JointId::HEAD_TILT;
}

bool bold::isArmJoint(JointId id)
{
  return id >= JointId::R_SHOULDER_PITCH && id <= JointId::L_ELBOW;
}

bool bold::isLegJoint(JointId id)
{
  return id >= JointId::R_HIP_YAW && id <= JointId::L_ANKLE_ROLL;
}
