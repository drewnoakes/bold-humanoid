#ifndef BOLD_LINE_SEGMENT_FINDER_HH
#define BOLD_LINE_SEGMENT_FINDER_HH

#include <opencv2/core/core.hpp>
#include <Eigen/Core>

#include "../Colour/colour.hh"
#include "../Geometry/geometry.hh"

namespace bold
{
  class LineFinder
  {
  private:
    cv::Mat d_mat;

    struct AccumulatorBin
    {
      double theta;
      double radius;
      ushort count;

      AccumulatorBin(double theta, double radius)
      : theta(theta),
        radius(radius),
        count(1)
      {}

      bool matches(double theta, double radius)
      {
        double dt = theta - this->theta;
        double dr = radius - this->radius;

        const double dtThreshold = 5/180.0 * M_PI;
        const double drThreshold = 5;

        return fabs(dt) < dtThreshold && fabs(dr) < drThreshold;
      }
    };

//     struct AccumulatorBinGroup
//     {
//       double theta;
//       double radius;
//       std::vector<Line> lines;
//     };

  public:
    LineFinder(int imageWidth, int imageHeight)
    : d_mat(imageWidth, imageHeight, CV_8UC1)
    {}

    std::vector<Line> find(std::vector<Eigen::Vector2i>& lineDots, ushort minCount = 15, ushort takeCount = 50)
    {
      // shuffle lineDots to simulate drawing at random
      std::random_shuffle(lineDots.begin(), lineDots.end());

      std::vector<Line> lines;
      std::vector<AccumulatorBin> bins;

      int dotIndex = std::min((size_t)takeCount, lineDots.size() - 1);

      while (dotIndex > 1)
      {
        auto dot1 = lineDots[dotIndex--];
        auto dot2 = lineDots[dotIndex--];

        if (dot1.x() == dot2.x() && dot1.y() == dot2.y())
          continue;

        auto segment = LineSegment2i(dot2, dot1);
        auto houghLine = segment.toLine();

        double theta = houghLine.theta();
        double radius = houghLine.radius();

        // Do we have an entry already for this?
        bool found = false;
        for (AccumulatorBin& bin : bins)
        {
          if (bin.matches(theta, radius))
          {
            // TODO reorder this and prior item(s) by vote count whenever increments occur
            bin.count++;
            found = true;
            break;
          }
        }

        // If not, create one
        if (!found)
        {
          bins.push_back(AccumulatorBin(theta, radius));
        }
      }

      for (AccumulatorBin& bin : bins)
      {
        if (bin.count > minCount)
          lines.push_back(Line(bin.radius, bin.theta, bin.count));
      }

      return lines;
    }
  };
}

#endif