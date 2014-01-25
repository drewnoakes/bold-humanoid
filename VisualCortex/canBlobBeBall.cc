#include "visualcortex.ih"

bool VisualCortex::canBlobBeBall(Blob const& blob, Vector2d* pos)
{
  auto body = AgentState::get<BodyState>();
  Affine3d const& cameraAgentTransform = body->getCameraAgentTransform();

  // At the point we think the ball is, find the distance in agent space between points left-to-right from there

  Rect rect = blob.toRect();
  int maxDimension = max(rect.width, rect.height);

  // TODO take the curvature of the ball into account -- project middle of blob on the plane z=ballRadius

  // Take the bottom of the ball as observation
  Vector2d basePos = blob.mean;
  basePos.y() = blob.ul.y();

  Vector2d sidePos = basePos + Vector2d(maxDimension/2, 0);

  auto midPointAgentSpace = d_spatialiser->findGroundPointForPixel(basePos, cameraAgentTransform);
  auto sidePointAgentSpace = d_spatialiser->findGroundPointForPixel(sidePos, cameraAgentTransform);

  if (!midPointAgentSpace || !sidePointAgentSpace)
    return false;

  double radiusAgentSpace = abs((*midPointAgentSpace - *sidePointAgentSpace).norm());

  static double ballRadius = Config::getStaticValue<double>("world.ball-diameter") / 2.0;

  // TODO VISION threshold in config
  double ballMeasuredSizeRatio = radiusAgentSpace / ballRadius;
  if (ballMeasuredSizeRatio < d_acceptedBallMeasuredSizeRatio->getValue().min() ||
      ballMeasuredSizeRatio > d_acceptedBallMeasuredSizeRatio->getValue().max())
    return false;

  *pos = basePos;
  return true;
}
