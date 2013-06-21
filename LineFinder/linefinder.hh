#pragma once

#include <vector>
#include <Eigen/Core>

#include "../Control/control.hh"
#include "../geometry/LineSegment2i.hh"

namespace bold
{
  class LineFinder
  {
  public:
    virtual ~LineFinder() {}

    virtual std::vector<LineSegment2i> findLineSegments(std::vector<Eigen::Vector2i>& lineDots) { return std::vector<LineSegment2i>(); };

    virtual std::vector<std::shared_ptr<Control const>> getControls() const { return std::vector<std::shared_ptr<Control const>>(); };
  };
}
