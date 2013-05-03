#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <cassert>
#include <cmath>

#include <Eigen/Core>
#include <opencv2/core/core.hpp>

#include "LineSegment.hh"
#include "Bounds.hh"
#include "Bounds2i.hh"
#include "LineSegment2i.hh"
#include "../util/Maybe.hh"

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
      assert(theta >= 0);
      assert(theta <= M_PI);
      assert(!std::isnan(theta) && !std::isnan(radius) && !std::isinf(theta) && !std::isinf(radius));
    };

    double radius() const { return d_radius; }
    double theta() const { return d_theta; }
    double thetaDegrees() const { return theta()*180.0/M_PI; }

    double gradient() const { return tanh(d_theta); }
    double yIntersection() const { return d_radius / cos(d_theta); }

    void draw(cv::Mat& mat, cv::Scalar const& color) const
    {
      Maybe<LineSegment<int,2>> line(intersectWith(Bounds2i(Eigen::Vector2i::Zero(), Eigen::Vector2i(mat.cols, mat.rows))));

      if (line.hasValue())
      {
        Eigen::Vector2i const& p1 = line.value().get()->p1();
        Eigen::Vector2i const& p2 = line.value().get()->p2();
        cv::line(mat, cv::Point(p1.x(), p1.y()), cv::Point(p2.x(), p2.y()), color);
      }
    }

    template<typename T>
    Maybe<LineSegment<T,2>> intersectWith(Bounds<T,2> bounds) const
    {
      assert(d_theta >= 0 && d_theta <= M_PI);

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
        edgeContactPoints.push_back(Eigen::Matrix<T,2,1>(xWhenYMin, minY));

      if (xWhenYMax >= minX && xWhenYMax <= maxX)
        edgeContactPoints.push_back(Eigen::Matrix<T,2,1>(xWhenYMax, maxY));

      if (yWhenXMin >= minY && yWhenXMin <= maxY)
        edgeContactPoints.push_back(Eigen::Matrix<T,2,1>(minX, yWhenXMin));

      if (yWhenXMax >= minY && yWhenXMax <= maxY)
        edgeContactPoints.push_back(Eigen::Matrix<T,2,1>(maxX, yWhenXMax));

      if (edgeContactPoints.size() == 0)
        return Maybe<LineSegment<T,2>>::empty();

      assert(edgeContactPoints.size() == 2);

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
