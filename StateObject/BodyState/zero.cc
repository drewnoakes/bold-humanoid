#include "bodystate.ih"

#include "../../MX28/mx28.hh"

shared_ptr<BodyState const> BodyState::zero(ulong thinkCycleNumber)
{
  array<double,23> angles;
  angles.fill(0);

  // Tilt the head up slightly, so that we can see the horizon in the image (better for testing)
  angles[(int)JointId::HEAD_TILT] = MX28::degs2Value(20.0);

  array<short,21> positionValueDiffs;
  positionValueDiffs.fill(0);

  return make_shared<BodyState>(angles, positionValueDiffs, thinkCycleNumber);
}
