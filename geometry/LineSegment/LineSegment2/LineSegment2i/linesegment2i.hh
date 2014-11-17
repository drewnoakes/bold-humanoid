#pragma once

#include <Eigen/Core>

#include "../../../../util/Maybe.hh"

#include "../../../Line.hh"
#include "../../../Bounds.hh"
#include "../linesegment2.hh"

namespace bold
{
  class Line;

  struct LineSegment2i : public LineSegment2<int>
  {
  public:
    LineSegment2i(LineSegment<int,2> const& other)
    : LineSegment2<int>::LineSegment2(other.p1(), other.p2())
    {}

    LineSegment2i(Eigen::Vector2i const& p1, Eigen::Vector2i const& p2)
    : LineSegment2<int>::LineSegment2(p1, p2)
    {}

    LineSegment2i(int const x1, int const y1, int const x2, int const y2)
    : LineSegment2<int>::LineSegment2(x1, y1, x2, y2)
    {}

    double gradient() const;

    double yIntersection() const;

    /** Returns the angle of this line to the +ve x-azis, in the range [-pi, pi] */
    double angle() const;

    /** Converts this {@link LineSegment2i} to a {@link Line}. */
    Line toLine() const;

    Maybe<LineSegment2i> cropTo(Bounds2i const& bounds) const;
  };
}
