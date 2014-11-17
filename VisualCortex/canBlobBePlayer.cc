#include "visualcortex.hh"

#include "../ImagePassHandler/BlobDetectPass/blobdetectpass.hh"
#include "../Spatialiser/spatialiser.hh"

using namespace bold;
using namespace Eigen;

bool VisualCortex::canBlobBePlayer(Blob const& playerBlob, Vector2d& imagePos, Vector3d& agentFramePos)
{
  //
  // Basic filtering
  //

  // Ignore blobs that are too small (avoid noise)
  // Also ignores blobs that were previously merged into another blob (zero area)
  if (playerBlob.area < unsigned(d_minPlayerAreaPixels->getValue()))
    return false;

  if (playerBlob.bounds().height() < d_minPlayerLengthPixels->getValue() ||
      playerBlob.bounds().width() < d_minPlayerLengthPixels->getValue())
    return false;

  // TODO the transform created in findGroundPointForPixel should only be created once (canBlobBePlayer is called in a loop)

  auto midPointAgentSpace = d_spatialiser->findGroundPointForPixel(playerBlob.mean.cast<double>(), d_goalieMarkerHeight->getValue());

  if (!midPointAgentSpace.hasValue())
    return false;

  imagePos = playerBlob.mean.cast<double>();
  agentFramePos = *midPointAgentSpace;

  return true;
}
