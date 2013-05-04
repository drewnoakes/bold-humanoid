#include "spatialiser.ih"

Maybe<Vector3d> Spatialiser::findGroundPointForPixel(Vector2d const& pixel, Affine3d const& cameraAgentTransform, double const groundZ) const
{
  Vector3d direction = cameraAgentTransform.rotation() * d_cameraModel->directionForPixel(pixel);

  Vector3d position = cameraAgentTransform.translation();

  return Math::intersectRayWithGroundPlane(position, direction, groundZ);
}

Maybe<Vector3d> Spatialiser::findGroundPointForPixel(Vector2d const& pixel, double const groundZ) const
{
  Affine3d cameraAgentTransform = AgentState::get<BodyState>()->getCameraAgentTransform();

  return findGroundPointForPixel(pixel, cameraAgentTransform, groundZ);
}
