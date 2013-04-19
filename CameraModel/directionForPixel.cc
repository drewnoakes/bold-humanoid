#include "cameramodel.ih"

Vector3d CameraModel::directionForPixel(Vector2i const& pixel) const
{
  Vector3d p;
  p << (pixel -Vector2i(d_imageWidth / 2, d_imageHeight / 2)).cast<double>(), 1;

  auto ptInv = getProjectionTransform().inverse();
  cout << ptInv.matrix() << endl;
  Vector3d dir(getProjectionTransform().inverse() * p);
  return dir.normalized();
}
