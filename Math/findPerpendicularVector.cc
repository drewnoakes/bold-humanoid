#include "math.ih"

Vector2d Math::findPerpendicularVector(Vector2d const& v)
{
  Vector3d v3d(v.x(), v.y(), 0.0);
  return v3d.cross(Vector3d::UnitZ()).head<2>();
}
