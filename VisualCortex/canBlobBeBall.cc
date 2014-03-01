#include "visualcortex.ih"

bool VisualCortex::canBlobBeBall(Blob const& blob, Vector2d* pos)
{
  //
  // Basic filtering
  //

  if (blob.area == 0)
  {
    // Ignore blobs that were previously merged into another blob (zero area)
    return false;
  }

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

  auto body = State::get<BodyState>(StateTime::CameraImage);
  Affine3d const& cameraAgentTransform = body->getCameraAgentTransform();

  // At the point we think the ball is, find the distance in agent space between points left-to-right from there

  Rect rect = blob.toRect();
  int maxDimension = max(rect.width, rect.height);

  Vector2d sidePos = blob.mean + Vector2d(maxDimension/2.0, 0);

  static double ballRadius = Config::getStaticValue<double>("world.ball-diameter") / 2.0;

  auto midPointAgentSpace = d_spatialiser->findGroundPointForPixel(blob.mean, cameraAgentTransform, ballRadius);
  auto sidePointAgentSpace = d_spatialiser->findGroundPointForPixel(sidePos, cameraAgentTransform, ballRadius);

  if (!midPointAgentSpace || !sidePointAgentSpace)
    return false;

  double radiusAgentSpace = abs((*midPointAgentSpace - *sidePointAgentSpace).norm());

  double ballMeasuredSizeRatio = radiusAgentSpace / ballRadius;
  if (ballMeasuredSizeRatio < d_acceptedBallMeasuredSizeRatio->getValue().min() ||
      ballMeasuredSizeRatio > d_acceptedBallMeasuredSizeRatio->getValue().max())
    return false;

  // If the ball would be further than the max diagonal distance of the field,
  // then we assume it is not the ball.
  if (midPointAgentSpace->norm() > d_fieldMap->getMaxDiagnoalFieldDistance())
    return false;

  *pos = blob.mean;
  return true;
}
