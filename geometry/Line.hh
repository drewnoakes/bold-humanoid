#pragma once

#include <iomanip>
#include <vector>
#include <cmath>

#include <Eigen/Core>
#include <opencv2/core/core.hpp>

#include "LineSegment/linesegment.hh"
#include "Bounds.hh"
#include "LineSegment/LineSegment2/LineSegment2i/linesegment2i.hh"
#include "../../Math/math.hh"
#include "../../util/assert.hh"
#include "../../util/Maybe.hh"

namespace bold
{
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
  // TODO rename to NormalLine2d?
  public:
    Line(double const radius, double const theta, const ushort votes = 0)
    : d_radius(radius),
      d_theta(theta)
    {
      ASSERT(theta >= 0);
      ASSERT(theta <= M_PI);
      ASSERT(!std::isnan(theta) && !std::isnan(radius) && !std::isinf(theta) && !std::isinf(radius));
    };

    double radius() const { return d_radius; }
    double theta() const { return d_theta; }
    double thetaDegrees() const { return Math::radToDeg(d_theta); }

    double gradient() const { return tanh(d_theta); }
    double yIntersection() const { return d_radius / cos(d_theta); }

    void draw(cv::Mat& mat, cv::Scalar const& color) const
    {
      Maybe<LineSegment<int,2>> line(intersectWith(Bounds2i(Eigen::Vector2i::Zero(), Eigen::Vector2i(mat.cols, mat.rows))));

      if (line.hasValue())
      {
        Eigen::Vector2i const& p1 = line->p1();
        Eigen::Vector2i const& p2 = line->p2();
        cv::line(mat, cv::Point(p1.x(), p1.y()), cv::Point(p2.x(), p2.y()), color);
      }
    }

    template<typename T>
    Maybe<LineSegment<T,2>> intersectWith(Bounds<T,2> bounds) const
    {
      ASSERT(d_theta >= 0 && d_theta <= M_PI);

      double tsin = sin(d_theta);
      double tcos = cos(d_theta);

      int minX = bounds.min().x();
      int minY = bounds.min().y();
      int maxX = bounds.max().x();
      int maxY = bounds.max().y();

      // r = x*sin(theta) + y*cos(theta)
      // x = (r - y*cos(theta))/sin(theta)
      // y = (r - x*sin(theta))/cos(theta)

      double xWhenYMin = (d_radius-minY*tcos)/tsin;
      double yWhenXMin = (d_radius-minX*tsin)/tcos;

      double xWhenYMax = (d_radius-maxY*tcos)/tsin;
      double yWhenXMax = (d_radius-maxX*tsin)/tcos;

      std::vector<Eigen::Matrix<T,2,1>> edgeContactPoints;

      if (xWhenYMin >= minX && xWhenYMin <= maxX)
        edgeContactPoints.emplace_back(xWhenYMin, minY);

      if (xWhenYMax >= minX && xWhenYMax <= maxX)
        edgeContactPoints.emplace_back(xWhenYMax, maxY);

      if (yWhenXMin >= minY && yWhenXMin <= maxY)
        edgeContactPoints.emplace_back(minX, yWhenXMin);

      if (yWhenXMax >= minY && yWhenXMax <= maxY)
        edgeContactPoints.emplace_back(maxX, yWhenXMax);

      if (edgeContactPoints.size() == 0)
        return Maybe<LineSegment<T,2>>::empty();

      ASSERT(edgeContactPoints.size() == 2);

      if (edgeContactPoints.size() != 2)
        return Maybe<LineSegment<T,2>>::empty();

      return Maybe<LineSegment<T,2>>(LineSegment<T,2>(edgeContactPoints[0], edgeContactPoints[1]));
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

  private:
    double d_radius;
    double d_theta;
  };
}
