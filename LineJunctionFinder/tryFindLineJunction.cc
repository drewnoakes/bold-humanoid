#include "linejunctionfinder.ih"

Maybe<pair<Vector2d, LineJunctionFinder::JunctionType>> LineJunctionFinder::tryFindLineJunction(LineSegment2d const& segment1, LineSegment2d const& segment2, double distToEndThreshold)
{
  auto length1 = segment1.length();
  auto length2 = segment2.length();

  double t{-1.0};
  double u{-1.0};

  segment1.tryIntersect(segment2, t, u);
  // Lines were parallel, no crossing
  if (t < 0)
    return Maybe<pair<Vector2d, JunctionType>>::empty();

  cout << "t: " << t << ", u: " << u << endl;

  bool on1 = t >= 0.0 && t <= 1.0;
  bool on2 = u >= 0.0 && u <= 1.0;

  auto distToEnd1 = std::min(std::abs(t), std::abs(1 - t)) * length1;
  auto distToEnd2 = std::min(std::abs(u), std::abs(1 - u)) * length2;

  bool atEnd1 = distToEnd1 < distToEndThreshold;
  bool atEnd2 = distToEnd2 < distToEndThreshold;

  JunctionType junction = JunctionType::NONE;

  if (on1 && on2 && !atEnd1 && !atEnd2)
    junction = JunctionType::X;
  else if ((on1 && !atEnd1 && atEnd2) ||
           (on2 && !atEnd2 && atEnd1))
    junction = JunctionType::T;
  else if (atEnd1 && atEnd2)
    junction = JunctionType::L;

  if (junction != JunctionType::NONE)
    return make_maybe(make_pair(Vector2d{segment1.p1() + segment1.delta() * t}, junction));
  else
    return Maybe<pair<Vector2d, JunctionType>>::empty();
}

