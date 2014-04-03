#include "bodystate.ih"

Affine3d BodyState::determineAgentFootTr(bool leftFoot) const
{
  auto torsoFootTr = getLimb(leftFoot ? "lFoot" :"rFoot")->transform.inverse();

  return Math::alignUp(torsoFootTr);
}
