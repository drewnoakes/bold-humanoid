#include "spatialiser.ih"

Maybe<Vector3d> Spatialiser::findGroundPointForPixel(Vector2d const& pixel, double const groundZ) const
{
  auto body = AgentState::get<BodyState>();

  Affine3d const& cameraAgentTransform = body->getCameraAgentTransform();

  return findGroundPointForPixel(pixel, cameraAgentTransform, groundZ);
}

Maybe<Vector3d> Spatialiser::findGroundPointForPixel(Vector2d const& pixel, Affine3d const& cameraAgentTransform, double const groundZ) const
{
  Vector3d direction = cameraAgentTransform.rotation() * d_cameraModel->directionForPixel(pixel);

  Vector3d position = cameraAgentTransform.translation();

  return Math::intersectRayWithGroundPlane(position, direction, groundZ);
}

Maybe<Vector2d> Spatialiser::findPixelForAgentPoint(Vector3d const& agentPoint) const
{
  Affine3d const& cameraAgentTransform = AgentState::get<BodyState>()->getCameraAgentTransform();
  return findPixelForAgentPoint(agentPoint, cameraAgentTransform);
}

Maybe<Vector2d> Spatialiser::findPixelForAgentPoint(Vector3d const& agentPoint, Affine3d const& cameraAgentTransform) const
{
  return d_cameraModel->pixelForDirection(cameraAgentTransform.inverse() * agentPoint);
}
