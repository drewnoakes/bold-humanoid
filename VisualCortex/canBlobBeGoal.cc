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
