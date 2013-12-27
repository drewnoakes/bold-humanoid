#pragma once

#include "LineSegment.hh"
#include "../util/Maybe.hh"

#include <Eigen/Core>

namespace bold
{
  template<typename T>
  class LineSegment2 : public LineSegment<T, 2>
  {
  public:
    typedef Eigen::Matrix<T, 2, 1> Point;

    LineSegment2(LineSegment<T,2> line)
    : LineSegment<T, 2>::LineSegment(line.p1(), line.p2())
    {}

    LineSegment2(Point const& p1, Point const& p2)
    : LineSegment<T, 2>::LineSegment(p1, p2)
    {}

    LineSegment2(T x1, T y1, T x2, T y2)
    : LineSegment<T, 2>::LineSegment(Point(x1, y1), Point(x2, y2))
    {}

    Maybe<Point> tryIntersect(LineSegment2<T> const& other) const
    {
      double t;
      double u;
      return tryIntersect(other, &t, &u);
    }

    Maybe<Point> tryIntersect(LineSegment2<T> const& other, double* t, double* u) const
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
        return Maybe<Point>::empty();
      }

      double t_numer = fake2dCross(pos2 - pos1, dir2);
      double u_numer = fake2dCross(pos2 - pos1, dir1);

      double tt = t_numer / denom;
      double uu = u_numer / denom;

      if (tt < 0 || tt > 1 || uu < 0 || uu > 1)
      {
        // line segments do not intersect within their ranges
        return Maybe<Point>::empty();
      }

      Eigen::Vector2d intersectionPoint = pos1 + dir1 * tt;

      *t = tt;
      *u = uu;

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
}
