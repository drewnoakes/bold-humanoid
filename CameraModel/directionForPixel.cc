#include "cameramodel.ih"

Vector3d CameraModel::directionForPixel(Vector2i const& pixel) const
{
  auto extremity = Vector3d(tan(rangeHorizontalRads()/2.0), 1, tan(rangeVerticalRads()/2.0));

  extremity.normalize();

  double xRatio = -((pixel.x() / (double)((d_imageWidth-1)/2.0)) - 1);
  double zRatio = (pixel.y() / (double)((d_imageHeight-1)/2.0)) - 1;

  extremity.x() *= xRatio;
  extremity.z() *= zRatio;

  return extremity.normalized();
}
