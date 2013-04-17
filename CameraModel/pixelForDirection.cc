#include "cameramodel.ih"

Vector2i CameraModel::pixelForDirection(Vector3d const& direction) const
{
  auto zero = Vector3d(tan(rangeHorizontalRads()/2.0), 1, -tan(rangeVerticalRads()/2.0));

  double xRatio = (d_imageWidth / 2.0) / zero.x();
  double yRatio = (d_imageHeight / 2.0) / zero.y();

  Vector2i pixel;
  pixel.x() = (direction.x() - zero.x()) * -xRatio;
  pixel.y() = (direction.z() - zero.z()) * yRatio;
  
  return pixel;
}
