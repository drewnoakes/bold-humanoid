#ifndef BOLD_LINE_HH
#define BOLD_LINE_HH

#include <iostream>
#include <iomanip>
#include <vector>
#include <cassert>
#include <cmath>

#include <Eigen/Core>
#include <opencv2/core/core.hpp>

namespace bold
{
  // TODO remove 'votes' from Line and make Scored<Line> template that augments any value with votes

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
}

#endif