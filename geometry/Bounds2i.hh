#ifndef BOLD_BOUNDS_2I_HH
#define BOLD_BOUNDS_2I_HH

#include <iostream>
#include <vector>

#include <Eigen/Core>
//#include <opencv2/core/core.hpp>

namespace bold
{
  struct LineSegment2i;

  class Bounds2i
  {
    // TODO add 'draw' for completeness

  public:
    Bounds2i(int minX, int minY, int maxX, int maxY);

    Bounds2i(Eigen::Vector2i min, Eigen::Vector2i max);

    bool contains(Eigen::Vector2i const& v) const;

    int width() const;

    int height() const;

    /** True, if the width or height of this Bounds2i is zero. */
    bool isEmpty() const;

    Eigen::Vector2i min() const { d_min; }
    Eigen::Vector2i max() const { d_max; }

    /** Returns corners in clockwise order, starting at 'min'. */
    std::vector<Eigen::Vector2i> getCorners() const;

    std::vector<LineSegment2i> getEdges() const;

    bool operator==(Bounds2i const& other) const
    {
      return d_min == other.d_min && d_max == other.d_max;
    }

    friend std::ostream& operator<<(std::ostream& stream, Bounds2i const& line)
    {
      return stream << "Bounds2i (min=" << line.d_min << " max=" << line.d_max << ")";
    }

  private:
    Eigen::Vector2i d_min;
    Eigen::Vector2i d_max;
  };
}

#endif