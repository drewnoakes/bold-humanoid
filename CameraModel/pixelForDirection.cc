#include "cameramodel.ih"

Maybe<Vector2d> CameraModel::pixelForDirection(Vector3d const& direction) const
{
  if (direction.y() < std::numeric_limits<double>::epsilon())
    return Maybe<Vector2d>::empty();

  // TODO: why doesn't it work when using p = T * d and then divide by p(2) instead?
  auto pixel = (d_projectionTransform * (direction / direction.y())); 
  return Maybe<Vector2d>(pixel.head<2>());
}
