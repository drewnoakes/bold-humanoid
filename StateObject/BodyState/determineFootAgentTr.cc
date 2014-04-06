#include "bodystate.ih"

Affine3d BodyState::determineFootAgentTr(bool leftFoot) const
{
  auto footTorsoTr = getLimb(leftFoot ? "lFoot" :"rFoot")->transform.inverse();

  return Math::alignUp(footTorsoTr);
}
