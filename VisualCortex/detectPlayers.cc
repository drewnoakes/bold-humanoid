#include "visualcortex.hh"

#include "../ImagePassHandler/BlobDetectPass/blobdetectpass.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

vector<Vector2d,aligned_allocator<Vector2d>> VisualCortex::detectPlayers(vector<Blob>& playerBlobs, SequentialTimer& t)
{
  vector<Vector2d,aligned_allocator<Vector2d>> playerPositions;
  for (auto const& playerBlob : playerBlobs)
  {
    Vector2d imagePos;
    Vector3d agentFramePos;
    if (!canBlobBePlayer(playerBlob, imagePos, agentFramePos))
      continue;

    playerPositions.push_back(imagePos);
  }
  return playerPositions;
}

