#include "linejunctionfinder.ih"

vector<pair<Vector2d, LineJunctionFinder::JunctionType>> LineJunctionFinder::findLineJunctions(vector<LineSegment2i> const& lineSegments)
{
  // Project all segments onto the ground
  auto pixels = MatrixXd{2, lineSegments.size() * 2};
  unsigned idx = 0;
  for (auto& pixel : lineSegments)
  {
    pixels.block<2,2>(0, idx) = pixel.cast<double>();
    idx += 2;
  }

  MatrixXd groundPoints;
  VectorXi valid;
  tie(groundPoints, valid) = d_spatialiser->findGroundPointsForPixels(pixels);

  // For each line segment pair, check where they intersect
  vector<pair<Vector2d, JunctionType>> junctions;

  for (unsigned i = 0; i < groundPoints.cols(); i += 2)
  {
    if (!valid(i))
      continue;
    auto segment1 = LineSegment2d{groundPoints.block<2,2>(0, i *2)};
    for (unsigned j = i + 2; j < groundPoints.cols(); j += 2)
    {
      if (!valid(j))
        continue;

      auto segment2 = LineSegment2d{groundPoints.block<2,2>(0, j *2)};
      auto junction = tryFindLineJunction(segment1, segment2);
      if (junction)
        junctions.push_back(*junction);
    }
  }

  return junctions;
}

