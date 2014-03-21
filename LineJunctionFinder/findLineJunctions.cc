#include "linejunctionfinder.ih"

vector<pair<Vector2d, LineJunctionFinder::JunctionType>> LineJunctionFinder::findLineJunctions(vector<LineSegment3d> const& lineSegments)
{
  // For each line segment pair, check where they intersect
  vector<pair<Vector2d, JunctionType>> junctions;

  for (unsigned i = 0; i < lineSegments.size(); ++i)
  {
    auto& segment1 = lineSegments[i];
    for (unsigned j = i + 1; j < lineSegments.size(); ++j)
    {
      auto& segment2 = lineSegments[j];
      auto junction = tryFindLineJunction(segment1, segment2);
      if (junction)
        junctions.push_back(*junction);
    }
  }

  return junctions;
}

