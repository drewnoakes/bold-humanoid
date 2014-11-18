#pragma once

#include <Eigen/Core>

#include "../../../../util/Maybe.hh"

#include "../../../Bounds.hh"
#include "../linesegment2.hh"

namespace bold
{
  class Line;

  struct LineSegment2i : public LineSegment2<int>
  {
  public:
    LineSegment2i(Eigen::Matrix<int, 2, 1> const& p1, Eigen::Matrix<int, 2, 1> const& p2)
    : LineSegment2<int>::LineSegment2(p1, p2)
    {}

    LineSegment2i(int x1, int y1, int x2, int y2)
    : LineSegment2<int>::LineSegment2(Point(x1, y1), Point(x2, y2))
    {}

    using LineSegment2<int>::LineSegment2;
    using LineSegment2<int>::operator=;

    double gradient() const;

    double yIntersection() const;

    /** Returns the angle of this line to the +ve x-azis, in the range [-pi, pi] */
    double angle() const;

    Maybe<LineSegment2i> cropTo(Bounds2i const& bounds) const;
  };
}
