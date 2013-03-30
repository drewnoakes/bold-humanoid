#include "math.hh"

using namespace bold;
using namespace Eigen;

Maybe<Vector3d> Math::intersectRayWithGroundPlane(Vector3d const& position,
                                                  Vector3d const& direction,
                                                  double const planeZ)
{
  if (direction.z() == 0)
    return Maybe<Vector3d>::empty();

  double t = - (position.z() - planeZ) / direction.z();

  if (t < 0)
    return Maybe<Vector3d>::empty();

  return Maybe<Vector3d>(position + (direction * t));
}

Maybe<Vector3d> Math::intersectRayWithPlane(Vector3d const& position,
                                            Vector3d const& direction,
                                            Vector4d const& plane)
{
  // x = pos.x + t * dir.x
  //
  // 0 = a * x + b * y + c * z + d = 0
  //
  // 0 = a * pos.x + a * t * dir.x +
  //     b * pos.y + b * t * dir.y +
  //     c * pos.z + c * t * dir.z +
  //     d;
  //
  // t * (a * dir.x + b * dir.y + c * dir.z) = - a * pos.x - b * pos.y - c * pos.z - d
  //
  // t = (- a * pos.x - b * pos.y - c * pos.z - d) / (a * dir.x + b * dir.y + c * dir.z)

  double denom = (plane.x() * direction.x() + plane.y() * direction.y() + plane.z() * direction.z());

  if (denom == 0)
    return Maybe<Vector3d>::empty();

  double t = - (plane.x() * position.x() + plane.y() * position.y() + plane.z() * position.z() + plane(3)) / denom;

  if (t < 0)
    return Maybe<Vector3d>::empty();

  return Maybe<Vector3d>(position + (direction * t));
}
