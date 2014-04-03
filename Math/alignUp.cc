#include "math.ih"

Affine3d Math::alignUp(Affine3d const& transform)
{
  auto tUp = transform.matrix().col(2).head<3>();
  auto nUp = Vector3d{0, 0, 1};

  auto rotationAngle = acos(tUp.dot(nUp));
  if (rotationAngle < 1e-6)
    return transform;

  auto rotationAxis = tUp.cross(nUp).normalized();

  Affine3d result = AngleAxisd(rotationAngle, rotationAxis) * transform;
  result.translation() = transform.translation();
  return result;
}
