#include "cameramodel.ih"

Vector2i CameraModel::pixelForDirection(Vector3d const& direction) const
{
  assert(direction.y() > 0);

  Vector3d p = getProjectionTransform() * direction;
  return (p / p.z()).cast<int>().head<2>() + Vector2i(d_imageWidth / 2, d_imageHeight / 2);
}
