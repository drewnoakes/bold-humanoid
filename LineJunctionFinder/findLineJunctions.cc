#include "linejunctionfinder.ih"

vector<LineJunction, aligned_allocator<LineJunction>> LineJunctionFinder::findLineJunctions(vector<LineSegment3d> const& lineSegments)
{
  // For each line segment pair, check where they intersect
  vector<LineJunction, aligned_allocator<LineJunction>> junctions;

  for (unsigned i = 0; i < static_cast<unsigned>(lineSegments.size()); ++i)
  {
    auto& segment1 = lineSegments[i];
    for (unsigned j = i + 1; j < static_cast<unsigned>(lineSegments.size()); ++j)
    {
      auto& segment2 = lineSegments[j];
      auto junction = tryFindLineJunction(segment1, segment2);
      if (junction)
      {
        auto angleDelta = std::abs(std::abs(junction->angle) - M_PI / 2);
        if (angleDelta > M_PI / 16)
          continue;
        junctions.push_back(*junction);
      }
    }
  }

  return junctions;
}

