#include "spatialiser.ih"

Maybe<Vector2d> Spatialiser::findPixelForAgentPoint(Vector3d const& agentPoint) const
{
  return findPixelForAgentPoint(agentPoint, State::get<BodyState>(StateTime::CameraImage)->getCameraAgentTransform());
}

Maybe<Vector2d> Spatialiser::findPixelForAgentPoint(Vector3d const& agentPoint, Affine3d const& cameraAgentTr) const
{
  return d_cameraModel->pixelForDirection(cameraAgentTr * agentPoint);
}
