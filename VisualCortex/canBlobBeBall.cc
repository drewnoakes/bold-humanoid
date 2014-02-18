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

  // TODO take the curvature of the ball into account -- project middle of blob on the plane z=ballRadius

  // Take the bottom of the ball as observation
  Vector2d basePos = blob.mean;
  basePos.y() = blob.ul.y();

  // Ignore ball if it appears outside the field edge
  //
  if (d_shouldIgnoreOutsideField->getValue() && basePos.y() > d_fieldEdgePass->getEdgeYValue(basePos.x()))
  {
    // This blob can not be the ball if its upper left corner is below the field edge.
    // Remember that the image appears upside down.
    return false;
  }

  //
  // Verify blob is about the expected pixel size at that position of the frame
  //

  auto body = AgentState::get<BodyState>(StateTime::CameraImage);
  Affine3d const& cameraAgentTransform = body->getCameraAgentTransform();

  // At the point we think the ball is, find the distance in agent space between points left-to-right from there

  Rect rect = blob.toRect();
  int maxDimension = max(rect.width, rect.height);

  Vector2d sidePos = basePos + Vector2d(maxDimension/2.0, 0);

  auto midPointAgentSpace = d_spatialiser->findGroundPointForPixel(basePos, cameraAgentTransform);
  auto sidePointAgentSpace = d_spatialiser->findGroundPointForPixel(sidePos, cameraAgentTransform);

  if (!midPointAgentSpace || !sidePointAgentSpace)
    return false;

  double radiusAgentSpace = abs((*midPointAgentSpace - *sidePointAgentSpace).norm());

  static double ballRadius = Config::getStaticValue<double>("world.ball-diameter") / 2.0;

  double ballMeasuredSizeRatio = radiusAgentSpace / ballRadius;
  if (ballMeasuredSizeRatio < d_acceptedBallMeasuredSizeRatio->getValue().min() ||
      ballMeasuredSizeRatio > d_acceptedBallMeasuredSizeRatio->getValue().max())
    return false;

  // If the ball would be further than the max diagonal distance of the field,
  // then we assume it is not the ball.
  if (midPointAgentSpace->norm() > d_fieldMap->getMaxDiagnoalFieldDistance())
    return false;

  *pos = basePos;
  return true;
}
