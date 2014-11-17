#include "visualcortex.hh"

using namespace bold;
using namespace Eigen;

bool VisualCortex::shouldMergeBallBlobs(Bounds2<ushort> const& larger, Bounds2<ushort> const& smaller)
{
  // Merge if the union would be more square, and the new maxDimension is not far off the original one

  // TODO VISION do not merge blobs if the union would be too large to be considered as the ball

  Bounds2<ushort> combined = Bounds2<ushort>::merge(larger, smaller);

  double largerAspect = (double)larger.minDimension() / larger.maxDimension();
  double combinedAspect = (double)combined.minDimension() / combined.maxDimension();

  // If combining these blobs would result in a shape that's less square-like, don't combine
  if (combinedAspect < largerAspect)
    return false;

  int maxDimension = std::max(larger.maxDimension(), smaller.maxDimension());
  int unionMaxDimension = combined.maxDimension();

  return unionMaxDimension < maxDimension * 1.3;
}
