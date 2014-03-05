#include "spatialiser.ih"

void Spatialiser::updateZeroGroundPixelTransform()
{
  updateZeroGroundPixelTransform(State::get<BodyState>(StateTime::CameraImage)->getAgentCameraTransform());
}

void Spatialiser::updateZeroGroundPixelTransform(Eigen::Affine3d const& agentCameraTr)
{
  d_zeroGroundPixelTr = findGroundPixelTransform(agentCameraTr, 0.0);
}
