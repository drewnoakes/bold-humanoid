#include "math.hh"

using namespace bold;
using namespace Eigen;

Maybe<Vector3d> Math::intersectRayWithGroundPlane(Vector3d const& position,
                                                  Vector3d const& direction,
                                                  double const planeZ)
{
  if (direction.z() == 0)
    return Maybe<Vector3d>::empty();

  double t = (planeZ - position.z()) / direction.z();

  if (t < 0)
    return Maybe<Vector3d>::empty();

  return Maybe<Vector3d>(position + (direction * t));
}