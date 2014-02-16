#include "visualcortex.ih"

bool VisualCortex::canBlobBeGoal(Blob const& blob, Vector2d* pos)
{
  auto body = AgentState::get<BodyState>(StateTime::CameraImage);
  Affine3d const& cameraAgentTransform = body->getCameraAgentTransform();

  // Find a measure of the width of the goal post in agent space

  // Take center of topmost run (the first)
  Run const& topRun = *blob.runs.begin();
  Vector2d basePos(
    (topRun.endX + topRun.startX) / 2.0,
    topRun.y
  );

  uint width = 0;
  for (Run const& run : blob.runs)
  {
    uint len = run.length();
    if (len > width)
      width = len;
  }

  Vector2d sidePos = basePos + Vector2d(width/2.0, 0);

  auto midPointAgentSpace = d_spatialiser->findGroundPointForPixel(basePos, cameraAgentTransform);
  auto sidePointAgentSpace = d_spatialiser->findGroundPointForPixel(sidePos, cameraAgentTransform);

  if (!midPointAgentSpace || !sidePointAgentSpace)
    return false;

  double radiusAgentSpace = abs((*midPointAgentSpace - *sidePointAgentSpace).norm());

  static double goalRadius = Config::getStaticValue<double>("world.goal-post-diameter") / 2.0;

  double goalMeasuredWidthRatio = radiusAgentSpace / goalRadius;
  if (goalMeasuredWidthRatio < d_acceptedGoalMeasuredWidthRatio->getValue().min() ||
      goalMeasuredWidthRatio > d_acceptedGoalMeasuredWidthRatio->getValue().max())
    return false;

  /*
  TODO test whether we're able to estimate the distance of a goal at the opposite end before enabling this

  // If the goal would be further than the max diagonal distance of the field,
  // then we assume it is not the ball.
  if (midPointAgentSpace->norm() > d_fieldMap->getMaxDiagnoalFieldDistance())
    return false;
  */

  *pos = basePos;
  return true;
}
