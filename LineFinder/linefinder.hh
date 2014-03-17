#pragma once

#include <vector>
#include <Eigen/Core>

#include "../geometry/LineSegment/LineSegment2/LineSegment2i/linesegment2i.hh"

namespace bold
{
  class LineFinder
  {
  public:
    virtual ~LineFinder() {}

    virtual std::vector<LineSegment2i> findLineSegments(std::vector<Eigen::Vector2i>& lineDots) { return std::vector<LineSegment2i>(); };
  };
}
