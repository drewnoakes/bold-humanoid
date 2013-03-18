#ifndef BOLD_BOUNDS_2I_HH
#define BOLD_BOUNDS_2I_HH

#include "Bounds.hh"

#include <vector>

#include <Eigen/Core>
//#include <opencv2/core/core.hpp>

namespace bold
{
  struct LineSegment2i;

  class Bounds2i : public Bounds<int,2>
  {
    // TODO add 'draw' for completeness

  public:
    Bounds2i(int minX, int minY, int maxX, int maxY)
    : Bounds<int,2>::Bounds(Eigen::Vector2i(minX, minY), Eigen::Vector2i(maxX, maxY))
    {}

    Bounds2i(Eigen::Vector2i min, Eigen::Vector2i max)
    : Bounds<int,2>::Bounds(min, max)
    {};

    int width() const;

    int height() const;

    /** Returns corners in clockwise order, starting at 'min'. */
    std::vector<Eigen::Vector2i> getCorners() const;

    std::vector<LineSegment2i> getEdges() const;
  };
}

#endif