#ifndef BOLD_LINE_SEGMENT_2I_HH
#define BOLD_LINE_SEGMENT_2I_HH

#include <Eigen/Core>
#include <opencv2/core/core.hpp>

#include "../util/Maybe.hh"
#include "../Colour/colour.hh"

#include "Line.hh"
#include "Bounds2i.hh"
#include "LineSegment.hh"

namespace bold
{
  class Line;

  struct LineSegment2i : public LineSegment<int,2>
  {
  public:
    LineSegment2i(LineSegment<int,2> const& other)
    : LineSegment<int,2>::LineSegment(other.p1(), other.p2())
    {};

    LineSegment2i(Eigen::Vector2i const& p1, Eigen::Vector2i const& p2)
    : LineSegment<int,2>::LineSegment(p1, p2)
    {};

    LineSegment2i(int const x1, int const y1, int const x2, int const y2)
    : LineSegment<int,2>::LineSegment(Eigen::Vector2i(x1, y1),
                                      Eigen::Vector2i(x2, y2))
    {};

    double gradient() const;

    double yIntersection() const;

    /** Returns the angle of this line to the +ve x-azis, in the range [-pi, pi] */
    double angle() const;

    void draw(cv::Mat& image, bold::Colour::bgr const& bgr) const;

    /** Converts this {@link LineSegment2i} to a {@link Line}. */
    Line toLine() const;

    Maybe<LineSegment2i> cropTo(Bounds2i const& bounds) const;

    Maybe<Eigen::Vector2i> tryIntersect(LineSegment2i const& other) const;

  private:
    /**
     * Returns the magnitude of the vector that would result from a regular
     * 3D cross product of the input vectors, taking their Z values implicitly
     * as 0 (i.e. treating the 2D space as a plane in the 3D space)
     */
    static double fake2dCross(Eigen::Vector2d const& a, Eigen::Vector2d const& b);
  };
}

#endif