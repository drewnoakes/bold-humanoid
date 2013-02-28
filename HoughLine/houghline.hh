#ifndef BOLD_HOUGHLINE_HH
#define BOLD_HOUGHLINE_HH

#include <vector>
#include <cassert>
#include <cmath>
#include <opencv2/core/core.hpp>

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
    ushort d_votes;

  public:
    HoughLine(double const radius, double const theta, const ushort votes)
      : d_radius(radius),
        d_theta(theta),
        d_votes(votes)
      {};

    double radius() const { return d_radius; }
    double theta() const { return d_theta; }
    ushort votes() const { return d_votes; }

    double gradient() const { return tanh(d_theta); }
    double yIntersection() const { return d_radius / cos(d_theta); }

    template <typename TPixel>
    void draw(cv::Mat& mat, TPixel color) const
    {
      int width = mat.cols; // +2 * offsetX;
      int height = mat.rows; // +2 * offsetY;

      double tsin = sin(d_theta);
      double tcos = cos(d_theta);

      // r = x*sin(theta) + y*cos(theta)

      // x = (r - y*cos(theta))/sin(theta)
      // y = (r - x*sin(theta))/cos(theta)

      assert(d_theta >= 0 && d_theta <= M_PI);

      if (d_theta < M_PI*0.25 || d_theta > M_PI*0.75)
      {
        // Draw horizontal-sh lines
        for (int x = 0; x < width; x++)
        {
          int y = (d_radius - x*tsin) / tcos;
          if (y >= 0 && y < height)
          {
            mat.at<TPixel>(y, x) = color;
          }
        }
      }
      else
      {
        // Draw vertical-ish lines
        for (int y = 0; y < height; y++)
        {
          int x = (d_radius - y*tcos) / tsin;
          if (x >= 0 && x < width)
          {
            mat.at<TPixel>(y, x) = color;
          }
        }
      }
    }
  };
}

#endif
