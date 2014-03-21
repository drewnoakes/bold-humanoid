#pragma once

#include <memory>
#include <vector>
#include <Eigen/Core>
#include "../geometry/LineSegment/LineSegment2/LineSegment2i/linesegment2i.hh"

namespace bold
{
  class Spatialiser;

  class LineJunctionFinder
  {
  public:
    enum class JunctionType
    {
      X,
      T,
        L,
        NONE
    };

    LineJunctionFinder() = default;

    std::vector<std::pair<Eigen::Vector2d, JunctionType>> findLineJunctions(std::vector<LineSegment3d> const& lineSegments);

    Maybe<std::pair<Eigen::Vector2d, JunctionType>> tryFindLineJunction(LineSegment3d const& segment1, LineSegment3d const& segment2, double distToEndThreshold = 0.2);
  };
}
