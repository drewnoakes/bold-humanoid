#include "spatialiser.ih"

Maybe<Vector3d> Spatialiser::findGroundPointForPixel(Vector2i const& pixel, Affine3d const& cameraGroundTransform, double const distanceAboveGround) const
{
  Vector3d direction = cameraGroundTransform.rotation() * d_cameraModel->directionForPixel(pixel);
  Vector3d position = cameraGroundTransform.translation();

  double torsoHeight = cameraGroundTransform.translation().z() - distanceAboveGround;

  return Math::intersectRayWithGroundPlane(position, direction, -torsoHeight);
}

Maybe<Vector3d> Spatialiser::findGroundPointForPixel(Vector2i const& pixel, double const distanceAboveGround) const
{
  Affine3d cameraGroundTransform = AgentState::get<BodyState>()->getCameraToGroundTransform();
  return findGroundPointForPixel(pixel, cameraGroundTransform, distanceAboveGround);
}
