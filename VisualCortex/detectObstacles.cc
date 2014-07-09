#include "visualcortex.ih"

vector<Obstacle> VisualCortex::detectObstacles(Maybe<Vector2d> const& ballPosition, Vector2dVector goalPositions, SequentialTimer& t) const
{
  auto deltas = d_fieldEdgePass->getEdgeDeltas();

  vector<Obstacle> obstacles;
  vector<pair<Vector2d,Vector2d>> points;

  auto makeObstacle = [&]
  {
    if (!points.empty())
    {
      Vector2d rightNear = points[0].first;
      Vector2d leftNear = points[points.size() - 1].first;

      obstacles.emplace_back();
    }
  };

  // Walk from right to left
  for (FieldEdgeDelta const& delta : deltas)
  {
    Vector2d nearAgent = d_spatialiser->findGroundPointForPixel(Vector2d(delta.x, delta.yRaw));
    Vector2d farAgent = d_spatialiser->findGroundPointForPixel(Vector2d(delta.x, delta.yConvex));

    // TODO review and consider alternative thresholds
    //

    bool isObstaclePoint = (farAgent - nearAgent).norm() < FieldMap::outerMarginMinimum();

    if (isObstaclePoint)
    {
      points.push_back(nearAgent, farAgent);
    }
    else
    {
      makeObstacle();
    }
  }

  makeObstacle();
}
