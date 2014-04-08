#include "visualcortex.ih"

bool VisualCortex::canBlobBeGoal(Blob const& blob, Vector2d& pos)
{
  // Find a measure of the width of the goal post in agent space

  // Take center of topmost run (the first) (bottom in image)
  Run const& topRun = *blob.runs.begin();
  Vector2d basePos(
    topRun.midX(),
    topRun.y
  );

  // Determine width as the average run length
  // TODO would be interesting to ensure low variance here, as well as just the average
  ulong widthSum = 0;
  for (Run const& run : blob.runs)
    widthSum += run.length();
  double width = widthSum / blob.runs.size();

  // Ensure the goal is rougly the expected radius (in metres)
  Vector2d sidePos = basePos + Vector2d(width/2.0, 0);

  auto midPointAgentSpace = d_spatialiser->findGroundPointForPixel(basePos);
  auto sidePointAgentSpace = d_spatialiser->findGroundPointForPixel(sidePos);

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
  if (midPointAgentSpace->norm() > FieldMap::getMaxDiagonalFieldDistance())
    return false;
  */

  pos = basePos;
  return true;
}
