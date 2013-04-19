#include "math.ih"

Eigen::Vector2d Math::linePointClosestToPoint(LineSegment2d const& segment, Eigen::Vector2d const &point)
{
  Vector2d const& lVect = segment.delta();

  Vector2d v = lVect.normalized();

  Vector2d s = findPerpendicularVector(v);

  Vector2d l0 = segment.p1();

  // There probably is a more efficient formula.
  double u;
  if (s.y() == 0)
    u = (l0.y() - point.y()) / -v.y();
  else
    u = ((s.x() / s.y())*(l0.y() - point.y()) + (point.x() - l0.x()))/(v.x() - (s.x() / s.y()) * v.y());

  // When no perpendicular line is posible within the segment, use
  // the closest endpoint.
  if (u > lVect.norm() || u < 0)
  {
    if ((l0 - point).norm() < ((l0 + lVect) - point).norm())
      return l0;
    else
      return l0+lVect;
  }

  return l0 + v*u;
}
