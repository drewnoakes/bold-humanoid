#include "bodystate.ih"

Affine3d BodyState::determineFootAgentTr(bool leftFoot) const
{
  auto footTorsoTr = getLimb(leftFoot ? "left-foot" :"right-foot")->transform.inverse();

  return Math::alignUp(footTorsoTr);
}
