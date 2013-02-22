#ifndef BOLD_HOUGHLINE_HH
#define BOLD_HOUGHLINE_HH

#include <vector>
#include <cmath>

namespace bold
{
  /**
   * Line, specified using radius/distance-from-origin parameters.
   */
  class HoughLine
  {
  private:
    double d_radius;
    double d_theta;

  public:
    HoughLine(double const radius, double const theta)
      : d_radius(radius),
        d_theta(theta)
      {};

    double radius() const { return d_radius; }
    double theta() const { return d_theta; }

    double gradient() const { return tanh(d_theta); }
    double yIntersection() const { return d_radius / cos(d_theta); }
  };
}

#endif
