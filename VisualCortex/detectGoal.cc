#include "visualcortex.hh"

#include "../ImagePassHandler/BlobDetectPass/blobdetectpass.hh"
#include "../ImagePassHandler/FieldEdgePass/fieldedgepass.hh"
#include "../SequentialTimer/sequentialtimer.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

vector<Vector2d,aligned_allocator<Vector2d>> VisualCortex::detectGoal(vector<Blob>& goalBlobs, SequentialTimer& t)
{
  vector<Vector2d,aligned_allocator<Vector2d>> goalPositions;

  vector<Blob,aligned_allocator<Blob>> acceptedGoalBlobs;
  int allowedGoalFieldEdgeDistPixels = d_maxGoalFieldEdgeDistPixels->getValue();
  int minGoalDimensionPixels = d_minGoalDimensionPixels->getValue();
  for (Blob const& goalBlob : goalBlobs)
  {
    // Ignore goal if it appears outside of field
    //
    // NOTE Process this before anything else as anything above the field edge is wasting our time
    if (goalBlob.ul.y() > int(d_fieldEdgePass->getEdgeYValue(goalBlob.mean.x())) + allowedGoalFieldEdgeDistPixels)
      continue;
    
    // TODO apply this filtering earlier, so that the debug image doesn't show unused goal blobs
    Matrix<ushort,2,1> wh = goalBlob.br - goalBlob.ul;
    
    if (wh.minCoeff() > minGoalDimensionPixels && // Ignore small blobs
        wh.y() > wh.x())                          // Taller than it is wide
    {
      // Verify this blob does not overlap with a goal blob which was already accepted
      for (auto const& other : acceptedGoalBlobs)
      {
        // Blobs are sorted by size, descending.
        // If a smaller goal blob intersects a larger blob that we already
        // accepted as a goal, ignore the smaller one.
        if (goalBlob.bounds().overlaps(other.bounds()))
          continue;
      }
      
      // Discard blobs that would be too wide/narrow for the goal we expect at that position of the frame
      Vector2d pos;
      if (!canBlobBeGoal(goalBlob, pos))
        continue;
      
      goalPositions.push_back(pos);
      acceptedGoalBlobs.push_back(goalBlob);
    }
  }
  t.timeEvent("Goal Blob Selection");

  if (log::minLevel <= LogLevel::Trace && acceptedGoalBlobs.size() > 2)
  {
    // It's pretty rare that we should see three goal posts, so log information about the blobs
    log::trace("VisualCortex::integrateImage") << acceptedGoalBlobs.size() << " accepted goal blobs";
    for (Blob const& goalBlob : acceptedGoalBlobs)
    {
      log::trace("VisualCortex::integrateImage")
        << goalBlob.br.x() << ","
        << goalBlob.br.y() << ","
        << goalBlob.ul.x() << ","
        << goalBlob.ul.y();
    }
  }

  return goalPositions;
}

