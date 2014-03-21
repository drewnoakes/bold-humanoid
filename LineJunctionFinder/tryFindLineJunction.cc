#include "linejunctionfinder.ih"

Maybe<LineJunction> LineJunctionFinder::tryFindLineJunction(LineSegment3d const& segment1, LineSegment3d const& segment2, double distToEndThreshold)
{
  LineSegment2d segment2d1 = segment1.to<2>();
  LineSegment2d segment2d2 = segment2.to<2>();

  auto length1 = segment2d1.length();
  auto length2 = segment2d2.length();

  double t{-1.0};
  double u{-1.0};

  segment2d1.tryIntersect(segment2d2, t, u);
  // Lines were parallel, no crossing
  if (t < 0)
    return Maybe<LineJunction>::empty();

  cout << "t: " << t << ", u: " << u << endl;

  bool on1 = t >= 0.0 && t <= 1.0;
  bool on2 = u >= 0.0 && u <= 1.0;

  auto distToEnd1 = std::min(std::abs(t), std::abs(1 - t)) * length1;
  auto distToEnd2 = std::min(std::abs(u), std::abs(1 - u)) * length2;

  bool atEnd1 = distToEnd1 < distToEndThreshold;
  bool atEnd2 = distToEnd2 < distToEndThreshold;

  LineJunction junction;
  junction.type = LineJunction::Type::NONE;

  if (on1 && on2 && !atEnd1 && !atEnd2)
    junction.type = LineJunction::Type::X;
  else if ((on1 && !atEnd1 && atEnd2) ||
           (on2 && !atEnd2 && atEnd1))
    junction.type = LineJunction::Type::T;
  else if (atEnd1 && atEnd2)
    junction.type = LineJunction::Type::L;

  if (junction.type != LineJunction::Type::NONE)
  {
    junction.position = Vector2d{segment2d1.p1() + segment2d1.delta() * t};
    junction.angle = segment2d1.smallestAngleBetween(segment2d2);
    return make_maybe(junction);
  }
  else
    return Maybe<LineJunction>::empty();
}

