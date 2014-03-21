#pragma once

#include <memory>
#include <Eigen/StdVector>
#include <Eigen/Core>
#include "../geometry/LineSegment/LineSegment2/LineSegment2i/linesegment2i.hh"

namespace bold
{
  class Spatialiser;

  struct LineJunction
  {
    enum class Type
    {
      X,
      T,
      L,
      NONE
    };

    Eigen::Vector2d position;
    Type type;
    double angle;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
  };

  class LineJunctionFinder
  {
  public:

    LineJunctionFinder() = default;

    std::vector<LineJunction, Eigen::aligned_allocator<LineJunction>> findLineJunctions(std::vector<LineSegment3d> const& lineSegments);

    Maybe<LineJunction> tryFindLineJunction(LineSegment3d const& segment1, LineSegment3d const& segment2, double distToEndThreshold = 0.2);
  };
}
