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

    LineJunctionFinder(std::shared_ptr<Spatialiser> spatialiser)
      : d_spatialiser(spatialiser)
    {}

    /** Find line junctions in agent frame given line segments in camera frame */
    std::vector<std::pair<Eigen::Vector2d, JunctionType>> findLineJunctions(std::vector<LineSegment2i> const& lineSegments);

    Maybe<std::pair<Eigen::Vector2d, JunctionType>> tryFindLineJunction(LineSegment2d const& segment1, LineSegment2d const& segment2, double distToEndThreshold = 0.2);

  private:
    std::shared_ptr<Spatialiser> d_spatialiser;

  };
}
