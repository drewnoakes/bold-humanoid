#include "spatialiser.ih"

Maybe<Vector3d> Spatialiser::findGroundPointForPixel(Vector2d const& pixel, double const groundZ) const
{
  auto body = State::get<BodyState>(StateTime::CameraImage);

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
  return findPixelForAgentPoint(agentPoint, State::get<BodyState>(StateTime::CameraImage)->getAgentCameraTransform());
}

Maybe<Vector2d> Spatialiser::findPixelForAgentPoint(Vector3d const& agentPoint, Affine3d const& agentCameraTransform) const
{
  return d_cameraModel->pixelForDirection(agentCameraTransform * agentPoint);
}
