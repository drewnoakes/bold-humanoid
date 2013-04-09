#include "spatialiser.ih"

Maybe<Vector3d> Spatialiser::findGroundPointForPixel(Vector2i const& pixel,
                                                     double const torsoHeight,
                                                     Affine3d const& cameraTorsoTransform) const
{
  Vector3d direction = cameraTorsoTransform.rotation() * d_cameraModel->directionForPixel(pixel);
  Vector3d position = cameraTorsoTransform.translation();// * Vector3d::Zero();
  return Math::intersectRayWithGroundPlane(position, direction, -torsoHeight);
}
