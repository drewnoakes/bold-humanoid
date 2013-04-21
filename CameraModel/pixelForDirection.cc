#include "cameramodel.ih"

Vector2i CameraModel::pixelForDirection(Vector3d const& direction) const
{
  assert(direction.y() > 0);

  // Projection in camera space (x to right, y forward and  up
  Vector3d p = (direction / direction.y()) * focalLength();

  // To image space: x to left, y up
  Vector2i pixel;
  pixel.x() = d_imageWidth / 2 * (-p.x() + 1);
  pixel.y() = d_imageHeight / 2 * (p.z() + 1);

  return pixel;
}
