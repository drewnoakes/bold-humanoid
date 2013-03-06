#ifndef BOLD_LINE_SEGMENT_FINDER_HH
#define BOLD_LINE_SEGMENT_FINDER_HH

#include <opencv2/core/core.hpp>
#include <Eigen/Core>

#include "../Colour/colour.hh"
#include "../geometry/Line.hh"
#include "../geometry/LineSegment2i.hh"

namespace bold
{
  class LineFinder
  {
  public:
    struct LineHypothesis
    {
      LineHypothesis(Line const& line, Eigen::Vector2i const& dot1, Eigen::Vector2i const& dot2)
      : d_theta(line.theta()),
        d_radius(line.radius())
      {
        auto diff = dot2 - dot1;

        // Determines whether we consider x or y for min/max
        d_isHorizontal = abs(diff.x()) > abs(diff.y());

        int elementIndex = d_isHorizontal ? 0 : 1;
        if (dot1[elementIndex] < dot2[elementIndex])
        {
          d_min = dot1;
          d_max = dot2;
        }
        else
        {
          d_min = dot2;
          d_max = dot1;
        }

        d_lines.push_back(line);
      }

      bool tryMerge(Line const& line, Eigen::Vector2i const& dot1, Eigen::Vector2i const& dot2)
      {
        double dt = line.theta() - d_theta;
        double dr = line.radius() - d_radius;

        const double dtThreshold = 5/180.0 * M_PI;
        const double drThreshold = 10;

        if (fabs(dt) < dtThreshold && fabs(dr) < drThreshold)
        {
          d_lines.push_back(line);

          // Update min/max values
          int elementIndex = d_isHorizontal ? 0 : 1;
          if (dot1[elementIndex] < dot2[elementIndex])
          {
            if (d_min[elementIndex] < dot1[elementIndex])
              d_min = dot1;
            if (d_max[elementIndex] > dot2[elementIndex])
              d_max = dot2;
          }
          else
          {
            if (d_min[elementIndex] < dot2[elementIndex])
              d_min = dot2;
            if (d_max[elementIndex] > dot1[elementIndex])
              d_max = dot1;
          }

          return true;
        }

        return false;
      }

      Line toLine() const
      {
        double t = d_theta;
        double r = d_radius;

        while (t < 0)
        {
          t += M_PI;
          r = -r;
        }

        while (t > M_PI)
        {
          t -= M_PI;
          r = -r;
        }

        return Line(r, t, d_lines.size());
      }

      int count() const
      {
        return d_lines.size();
      }

      Eigen::Vector2i min() const { return d_min; }
      Eigen::Vector2i max() const { return d_max; }

    private:
      Eigen::Vector2i d_min;
      Eigen::Vector2i d_max;
      double d_theta;
      double d_radius;
      std::vector<Line> d_lines;
      bool d_isHorizontal;
    };

  public:
    LineFinder(int imageWidth, int imageHeight)
    : d_mat(imageWidth, imageHeight, CV_8UC1)
    {}

    std::vector<LineHypothesis> find(std::vector<Eigen::Vector2i>& lineDots, ushort processDotCount = 50)
    {
      std::vector<LineHypothesis> hypotheses;

      // shuffle lineDots to simulate drawing at random
      std::random_shuffle(lineDots.begin(), lineDots.end());

      int dotIndex = std::min((size_t)processDotCount, lineDots.size() - 1);

      while (dotIndex > 1)
      {
        auto dot1 = lineDots[dotIndex--];
        auto dot2 = lineDots[dotIndex--];

        if (dot1.x() == dot2.x() && dot1.y() == dot2.y())
          continue;

        auto segment = LineSegment2i(dot1, dot2);
        auto line = segment.toLine();

        // Do we have an entry already for this?
        bool found = false;
        for (LineHypothesis& hypothesis : hypotheses)
        {
          if (hypothesis.tryMerge(line, dot1, dot2))
          {
            found = true;
            break;
          }
        }

        // If not, create one
        if (!found)
        {
          hypotheses.push_back(LineHypothesis(line, dot1, dot2));
        }
      }

      // Sort highest-voted first
      std::sort(hypotheses.begin(), hypotheses.end(), [](LineHypothesis const& a, LineHypothesis const& b) { return a.count() > b.count(); });

      return hypotheses;
    }

  private:
    cv::Mat d_mat;
  };
}

#endif