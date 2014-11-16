#include "cameramodel.hh"

using namespace bold;
using namespace std;
using namespace Eigen;

Maybe<Vector2d> CameraModel::pixelForDirection(Vector3d const& direction) const
{
  if (direction.y() < std::numeric_limits<double>::epsilon())
    return Maybe<Vector2d>::empty();

  // TODO: why doesn't it work when using p = T * d and then divide by p(2) instead?
  Vector4d hDirection{0,0,0,1};
  hDirection.head<3>() = direction;

  auto pixel = Vector4d{d_projectionTransform * hDirection};
  return Maybe<Vector2d>(pixel.head<2>() / pixel(2) + Vector2d{d_imageWidth / 2.0, d_imageHeight / 2.0});
}
