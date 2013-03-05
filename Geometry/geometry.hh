#ifndef BOLD_GEOMETRY_HH
#define BOLD_GEOMETRY_HH

#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <memory>

#include <Eigen/Core>
#include <opencv2/core/core.hpp>

#include "../Colour/colour.hh"

namespace bold
{
  // TODO remove 'votes' from Line and make Scored<Line> template that augments any value with votes

  template<typename T>
  struct Maybe
  {
    bool hasValue() { return d_hasValue; }
    std::shared_ptr<T> value() { return d_value; }

    static Maybe<T> empty() { return Maybe<T>(false); }

    Maybe(T value)
    : d_hasValue(true),
      d_value(std::make_shared<T>(value))
    {}

    Maybe(std::shared_ptr<T> value)
    : d_hasValue(true),
      d_value(value)
    {}

    bool operator==(Maybe const& other) const
    {
      if (d_hasValue ^ other.d_hasValue)
        return false;

      return !d_hasValue || *d_value == *other.d_value;
    }

    friend std::ostream& operator<<(std::ostream& stream, Maybe<T> const& maybe)
    {
      if (maybe.d_hasValue)
      {
        return stream << "Maybe (hasValue=true value=" << *maybe.d_value << ")";
      }
      return stream << "Maybe (hasValue=false)";
    }

  private:
    Maybe(bool hasValue)
    : d_hasValue(false),
      d_value(0)
    {}

    bool d_hasValue;
    std::shared_ptr<T> d_value;
  };

//   template<typename TItem>
//   struct Scored
//   {
//     TItem item;
//     int value;
//
//     Scored(TItem item, TValue value)
//     : item(item), value(value)
//     {}
//   };

  /**
   * Line, specified using radius/distance-from-origin parameters (normal form.)
   *
   * Equations defining the line are:
   *
   *   r = x*sin(theta) + y*cos(theta)
   *
   *   x = (r - y*cos(theta))/sin(theta)
   *   y = (r - x*sin(theta))/cos(theta)
   */
  class Line
  {
    // TODO rename to NormalLine?
  private:
    double d_radius;
    double d_theta;
    ushort d_votes;

  public:
    Line(double const radius, double const theta, const ushort votes = 0)
    : d_radius(radius),
      d_theta(theta),
      d_votes(votes)
    {
      assert(theta >= 0);
      assert(theta <= M_PI);
      assert(!isnan(theta) && !isnan(radius) && !isinf(theta) && !isinf(radius));
    };

    double radius() const { return d_radius; }
    double theta() const { return d_theta; }
    ushort votes() const { return d_votes; }
    double thetaDegrees() const { return theta()*180.0/M_PI; }

    double gradient() const { return tanh(d_theta); }
    double yIntersection() const { return d_radius / cos(d_theta); }

    void draw(cv::Mat& mat, cv::Scalar const& color) const
    {
      assert(d_theta >= 0 && d_theta <= M_PI);

      double tsin = sin(d_theta);
      double tcos = cos(d_theta);

      int maxY = mat.rows - 1;
      int maxX = mat.cols - 1;

      // r = x*sin(theta) + y*cos(theta)
      // x = (r - y*cos(theta))/sin(theta)
      // y = (r - x*sin(theta))/cos(theta)

      double xWhenYZero = d_radius/tsin;
      double yWhenXZero = d_radius/tcos;

      double xWhenYMax = (d_radius-maxY*tcos)/tsin;
      double yWhenXMax = (d_radius-maxX*tsin)/tcos;

      std::vector<cv::Point> edgeContactPoints;

      if (xWhenYZero >= 0 && xWhenYZero <= maxX)
        edgeContactPoints.push_back(cv::Point(xWhenYZero, 0));

      if (xWhenYMax >= 0 && xWhenYMax <= maxX)
        edgeContactPoints.push_back(cv::Point(xWhenYMax, maxY));

      if (yWhenXZero >= 0 && yWhenXZero <= maxY)
        edgeContactPoints.push_back(cv::Point(0, yWhenXZero));

      if (yWhenXMax >= 0 && yWhenXMax <= maxY)
        edgeContactPoints.push_back(cv::Point(maxX, yWhenXMax));

      if (edgeContactPoints.size() == 0)
        return;

      assert(edgeContactPoints.size() == 2);

      if (edgeContactPoints.size() != 2)
        return;

      cv::line(mat, edgeContactPoints[0], edgeContactPoints[1], color);
    }

    bool operator==(Line const& other) const
    {
      const double epsilon = 0.0000001;
      return fabs(d_radius - other.d_radius) < epsilon
          && fabs(d_theta - other.d_theta) < epsilon;
    }

    friend std::ostream& operator<<(std::ostream& stream, Line const& line)
    {
      return stream << std::setprecision(13) << "Line (radius=" << line.radius() << " theta=" << line.theta() << ")";
    }
  };

  class Bounds2i
  {
  public:
    Bounds2i(int minX, int minY, int maxX, int maxY)
    : d_min(Eigen::Vector2i(minX, minY)),
      d_max(Eigen::Vector2i(maxX, maxY))
    {
      assert(d_min.x() <= d_max.x() && d_min.y() <= d_max.y());
    }

    Bounds2i(Eigen::Vector2i min, Eigen::Vector2i max)
    : d_min(min),
      d_max(max)
    {
      assert(d_min.x() <= d_max.x() && d_min.y() <= d_max.y());
    }

    bool contains(Eigen::Vector2i const& v) const
    {
      return v.x() >= d_min.x()
          && v.x() <= d_max.x()
          && v.y() >= d_min.y()
          && v.y() <= d_max.y();
    }

  private:
    Eigen::Vector2i d_min;
    Eigen::Vector2i d_max;
  };

  struct LineSegment2i
  {
  private:
    Eigen::Vector2i d_p1;
    Eigen::Vector2i d_p2;

    /**
     * Returns the magnitude of the vector that would result from a regular
     * 3D cross product of the input vectors, taking their Z values implicitly
     * as 0 (i.e. treating the 2D space as a plane in the 3D space)
     */
    static double fake2dCross(Eigen::Vector2d const& a, Eigen::Vector2d const& b)
    {
      return a.x()*b.y() - a.y()*b.x();
    }

  public:
    LineSegment2i(Eigen::Vector2i p1, Eigen::Vector2i p2)
    : d_p1(p1), d_p2(p2)
    {
      if (p1.x() == p2.x() && p1.y() == p2.y())
        throw std::string("Points must have different values.");
    }

    LineSegment2i(int x1, int y1, int x2, int y2)
    : d_p1(Eigen::Vector2i(x1, y1)), d_p2(Eigen::Vector2i(x2, y2))
    {
      if (x1 == x2 && y1 == y2)
        throw std::string("Points must have different values.");
    }

    Eigen::Vector2i p1() const { return d_p1; }
    Eigen::Vector2i p2() const { return d_p2; }

    /** Returns the vector formed by p2() - p1() */
    Eigen::Vector2i delta() const { return d_p2 - d_p1; }

    double gradient() const
    {
      auto d = delta();
      if (d.x() == 0)
        return FP_INFINITE;
      return d.y() / (double)d.x();
    }

    double yIntersection() const
    {
      if (d_p1.x() == d_p2.x())
        return FP_NAN;
      return (double)d_p1.y() - gradient() * (double)d_p1.x();
    }

    /** Returns the angle of this line to the +ve x-azis, in the range [-pi, pi] */
    double angle() const
    {
      auto delta = this->delta();
      return atan2(delta.y(), delta.x());
    }

    void draw(cv::Mat& image, bold::Colour::bgr const& bgr) const
    {
      cv::line(image, cv::Point(d_p1.x(), d_p1.y()), cv::Point(d_p2.x(), d_p2.y()), bgr.toScalar());
    }

    /** Converts this {@link LineSegment2i} to a {@link Line}. */
    Line toLine() const
    {
      double theta = atan2(d_p2.y() - d_p1.y(), d_p1.x() - d_p2.x());

      double radius = d_p1.x()*sin(theta) + d_p1.y()*cos(theta);

      while (theta < 0)
      {
        theta += M_PI;
        radius = -radius;
      }

      while (theta > M_PI)
      {
        theta -= M_PI;
        radius = -radius;
      }

      return Line(radius, theta);
    }

    LineSegment2i cropTo(Bounds2i const& bounds) const
    {
      return *this;
    }

    Maybe<Eigen::Vector2i> tryIntersect(LineSegment2i const& other)
    {
      // http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect

      Eigen::Vector2d pos1 = d_p1.cast<double>(); // p
      Eigen::Vector2d dir1 = delta().cast<double>(); // r
      Eigen::Vector2d pos2 = other.d_p1.cast<double>(); // q
      Eigen::Vector2d dir2 = other.delta().cast<double>(); // s

      double denom = fake2dCross(dir1, dir2);
      double numer = fake2dCross(pos2 - pos1, dir2);

      if (/*numer == 0 ||*/ denom == 0)
      {
        // lines are collinear or parallel
        return Maybe<Eigen::Vector2i>::empty();
      }

      double t = numer / denom;

      if (t < 0 || t > 1)
      {
        // line segments do not intersect within their ranges
        return Maybe<Eigen::Vector2i>::empty();
      }

      auto intersectionPoint = pos1 + dir1 * t;

      return Maybe<Eigen::Vector2i>(intersectionPoint.cast<int>());
    }

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
  };
}

#endif
