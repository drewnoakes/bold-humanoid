#pragma once

#include "../linesegment.hh"
#include "../../../util/Maybe.hh"

#include <Eigen/Core>

namespace bold
{
  template<typename T>
  class LineSegment2 : public LineSegment<T, 2>
  {
  public:
    typedef Eigen::Matrix<T,2,1> Point;

    LineSegment2(Point const& p1, Point const& p2)
    : LineSegment<T, 2>::LineSegment(p1, p2)
    {}

    LineSegment2(T x1, T y1, T x2, T y2)
    : LineSegment<T, 2>::LineSegment(Point(x1, y1), Point(x2, y2))
    {}

    using LineSegment<T, 2>::LineSegment;
    using LineSegment<T, 2>::operator=;

    Maybe<Point> tryIntersect(LineSegment2<T> const& other) const
    {
      double t;
      double u;
      return tryIntersect(other, t, u);
    }

    /**
    * Attempt to intersect two line segments.
    * Note that even if the line segments do not intersect, the t and u values will be set.
    * @param other the line to attempt intersection of this line with
    * @param t (output) the distance along this line at which intersection would occur, or NaN if lines are collinear/parallel
    * @param u (output) the distance along the other line at which intersection would occur, or NaN if lines are collinear/parallel
    * @return The point of intersection if within the line segments, or empty.
    */
    Maybe<Point> tryIntersect(LineSegment2<T> const& other, double& t, double& u) const
    {
      // http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect

      Eigen::Vector2d pos1 = this->p1().template cast<double>();    // p
      Eigen::Vector2d pos2 = other.p1().template cast<double>();    // q
      Eigen::Vector2d dir1 = this->delta().template cast<double>(); // r
      Eigen::Vector2d dir2 = other.delta().template cast<double>(); // s

      // t = (q − p) × s / (r × s)
      // u = (q − p) × r / (r × s)

      double denom = fake2dCross(dir1, dir2);

      if (denom == 0)
      {
        // lines are collinear or parallel
        t = std::numeric_limits<double>::quiet_NaN();
        u = std::numeric_limits<double>::quiet_NaN();
        return Maybe<Point>::empty();
      }

      double t_numer = fake2dCross(pos2 - pos1, dir2);
      double u_numer = fake2dCross(pos2 - pos1, dir1);

      t = t_numer / denom;
      u = u_numer / denom;

      if (t < 0 || t > 1 || u < 0 || u > 1)
      {
        // line segments do not intersect within their ranges
        return Maybe<Point>::empty();
      }

      Eigen::Vector2d intersectionPoint = pos1 + dir1 * t;

      // If we are using integers, be sure to round the result before casting
      if (std::is_same<T,int>())
      {
        Eigen::Vector2d rounded = intersectionPoint.unaryExpr([](double v) { return round(v); });
        return Maybe<Point>(rounded.cast<T>());
      }

      return Maybe<Point>(intersectionPoint.cast<T>());
    }

  private:
    /**
     * Returns the magnitude of the vector that would result from a regular
     * 3D cross product of the input vectors, taking their Z values implicitly
     * as 0 (i.e. treating the 2D space as a plane in the 3D space)
     */
    inline static double fake2dCross(Eigen::Vector2d const& a, Eigen::Vector2d const& b)
    {
      return a.x()*b.y() - a.y()*b.x();
    }
  };

  typedef LineSegment2<double> LineSegment2d;
  typedef LineSegment2<float> LineSegment2f;
}
