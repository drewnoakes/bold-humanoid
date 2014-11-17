#include "visualcortex.hh"

#include "../FieldMap/fieldmap.hh"
#include "../ImagePassHandler/BlobDetectPass/blobdetectpass.hh"
#include "../ImagePassHandler/FieldEdgePass/fieldedgepass.hh"
#include "../Spatialiser/spatialiser.hh"

using namespace bold;
using namespace cv;
using namespace Eigen;
using namespace std;

bool VisualCortex::canBlobBeBall(Blob const& blob, Vector2d& imagePos, Vector3d& agentFramePos)
{
  //
  // Basic filtering
  //

  // Ignore balls that are too small (avoid noise)
  // Also ignores blobs that were previously merged into another blob (zero area)
  if (blob.area < unsigned(d_minBallAreaPixels->getValue()))
    return false;

  // Ignore ball if it is too far from the field edge
  //
  int allowedBallFieldEdgeDistPixels = d_maxBallFieldEdgeDistPixels->getValue();
  if (blob.ul.y() > int(d_fieldEdgePass->getEdgeYValue(blob.mean.x())) + allowedBallFieldEdgeDistPixels)
  {
    // This blob can not be the ball if its upper left corner is too far below the field edge.
    // Remember that the image appears upside down.
    return false;
  }

  //
  // Verify blob is about the expected pixel size at that position of the frame
  //

  // At the point we think the ball is, find the distance in agent space between points left-to-right from there

  Rect rect = blob.toRect();
  int maxDimension = max(rect.width, rect.height);

  Vector2d sidePos = blob.mean.cast<double>() + Vector2d(maxDimension/2.0, 0);

  static double ballRadius = FieldMap::getBallRadius();

  auto midPointAgentSpace = d_spatialiser->findGroundPointForPixel(blob.mean.cast<double>(), ballRadius);
  auto sidePointAgentSpace = d_spatialiser->findGroundPointForPixel(sidePos, ballRadius);

  if (!midPointAgentSpace || !sidePointAgentSpace)
    return false;

  double radiusAgentSpace = (*midPointAgentSpace - *sidePointAgentSpace).norm();

  double ballMeasuredSizeRatio = radiusAgentSpace / ballRadius;
  if (ballMeasuredSizeRatio < d_acceptedBallMeasuredSizeRatio->getValue().min() ||
      ballMeasuredSizeRatio > d_acceptedBallMeasuredSizeRatio->getValue().max())
    return false;

  // If the ball would be further than the max diagonal distance of the field,
  // then we assume it is not the ball.
  if (midPointAgentSpace->norm() > FieldMap::getMaxDiagonalFieldDistance())
    return false;

  imagePos = blob.mean.cast<double>();
  agentFramePos = *midPointAgentSpace;
  return true;
}
