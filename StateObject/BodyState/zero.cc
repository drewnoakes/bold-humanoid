#include "bodystate.ih"

#include "../../MX28/mx28.hh"

shared_ptr<BodyState const> BodyState::zero(ulong thinkCycleNumber)
{
  double angles[(int)JointId::MAX + 1] = {0,};

  // Tilt the head up slightly, so that we can see the horizon in the image (better for testing)
  angles[(int)JointId::HEAD_TILT] = MX28::degs2Value(20.0);

  std::vector<int> positionValueDiffs((int)JointId::MAX + 2, 0);

  return make_shared<BodyState>(angles, positionValueDiffs, thinkCycleNumber);
}
