#ifndef BOLD_LINE_SEGMENT_2I_HH
#define BOLD_LINE_SEGMENT_2I_HH

#include <Eigen/Core>
#include <opencv2/core/core.hpp>

#include "../util/Maybe.hh"
#include "../Colour/colour.hh"

#include "Line.hh"
#include "Bounds2i.hh"

namespace bold
{
  struct LineSegment2i
  {
  public:
    LineSegment2i(Eigen::Vector2i p1, Eigen::Vector2i p2);

    LineSegment2i(int x1, int y1, int x2, int y2);

    Eigen::Vector2i p1() const { return d_p1; }
    Eigen::Vector2i p2() const { return d_p2; }

    /** Returns the vector formed by p2() - p1() */
    Eigen::Vector2i delta() const { return d_p2 - d_p1; }

    double gradient() const;

    double yIntersection() const;

    /** Returns the angle of this line to the +ve x-azis, in the range [-pi, pi] */
    double angle() const;

    void draw(cv::Mat& image, bold::Colour::bgr const& bgr) const;

    /** Converts this {@link LineSegment2i} to a {@link Line}. */
    Line toLine() const;

    Maybe<LineSegment2i> cropTo(Bounds2i const& bounds) const;

    Maybe<Eigen::Vector2i> tryIntersect(LineSegment2i const& other) const;

    bool operator==(LineSegment2i const& other) const
    {
      const double epsilon = 0.0000001;
      return fabs(d_p1.x() - other.d_p1.x()) < epsilon
          && fabs(d_p1.y() - other.d_p1.y()) < epsilon
          && fabs(d_p2.x() - other.d_p2.x()) < epsilon
          && fabs(d_p2.y() - other.d_p2.y()) < epsilon;
    }

    friend std::ostream& operator<<(std::ostream& stream, LineSegment2i const& lineSegment)
    {
      return stream << "LineSegment2i (P1=" << lineSegment.d_p1 << " P2=" << lineSegment.d_p2 << ")";
    }

  private:
    Eigen::Vector2i d_p1;
    Eigen::Vector2i d_p2;

    /**
     * Returns the magnitude of the vector that would result from a regular
     * 3D cross product of the input vectors, taking their Z values implicitly
     * as 0 (i.e. treating the 2D space as a plane in the 3D space)
     */
    static double fake2dCross(Eigen::Vector2d const& a, Eigen::Vector2d const& b);
  };
}

#endif