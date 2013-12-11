#include "bodystate.ih"

shared_ptr<BodyState const> BodyState::zero(ulong thinkCycleNumber)
{
  double angles[(int)JointId::MAX + 1] = {0,};
  return make_shared<BodyState>(angles, thinkCycleNumber);
}
