#include "cameramodel.ih"

Vector3d CameraModel::directionForPixel(Vector2i pixel) const
{
  auto extremity = Vector3d(tan(d_rangeHorizontal/2.0), tan(d_rangeVertical/2.0), 1);

  extremity.normalize();

  double xRatio = (pixel.x() / (double)((d_imageWidth-1)/2.0)) - 1;
  double yRatio = (pixel.y() / (double)((d_imageHeight-1)/2.0)) - 1;

  extremity.x() *= xRatio;
  extremity.y() *= yRatio;

  return extremity.normalized();
}