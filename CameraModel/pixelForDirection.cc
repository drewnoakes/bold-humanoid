#include "cameramodel.ih"

Vector2i CameraModel::pixelForDirection(Vector3d const& direction) const
{
  assert(direction.y() > 0);

  double r = focalLength() * tan(rangeVerticalRads() / 2);

  // Projection in camera space (x to right, y forward and  up
  Vector3d p = (direction / direction.y()) * focalLength();

  // To image space: x to left, y up
  Vector2i pixel;
  pixel.x() = (d_imageWidth - 1) / 2 * (-p.x() + 1) + 0.5;
  pixel.y() = (d_imageHeight - 1) / 2 * (p.z() / r + 1) + 0.5;

  return pixel;
}
