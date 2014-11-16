#include "cameramodel.hh"

using namespace bold;
using namespace std;
using namespace Eigen;

Vector3d CameraModel::directionForPixel(Vector2d const& pixel) const
{
  double r = focalLength() * tan(rangeVerticalRads() / 2);

  Vector3d p(
    (-2.0 * pixel.x() / d_imageWidth) + 1.0,
    focalLength(),
    ((2.0 * pixel.y() / d_imageHeight) - 1.0) * r
  );

  Vector3d dir = p / focalLength();

  return dir.normalized();
}
