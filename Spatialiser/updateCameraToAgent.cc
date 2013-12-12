#include "spatialiser.ih"

void Spatialiser::updateCameraToAgent()
{
  auto cameraFrame = AgentState::get<CameraFrameState>();

  auto const& ballObs = cameraFrame->getBallObservation();

  Maybe<Vector3d> ball = ballObs.hasValue()
    ? findGroundPointForPixel(*ballObs)
    : Maybe<Vector3d>::empty();

  std::vector<Vector3d> goals;
  std::vector<LineSegment3d> lineSegments;

  for (Vector2d const& goal : cameraFrame->getGoalObservations())
  {
    auto const& pos3d = findGroundPointForPixel(goal);
    if (pos3d.hasValue())
    {
      goals.push_back(*pos3d);
    }
  }

  for (LineSegment2i const& lineSegment : cameraFrame->getObservedLineSegments())
  {
    auto const& p1 = findGroundPointForPixel(lineSegment.p1().cast<double>() + Vector2d(0.5,0.5));
    auto const& p2 = findGroundPointForPixel(lineSegment.p2().cast<double>() + Vector2d(0.5,0.5));
    if (p1.hasValue() && p2.hasValue())
    {
      lineSegments.push_back(LineSegment3d(*p1, *p2));
    }
  }

  // Determine observed field area polygon
  vector<Vector3d> visibleFieldPoly;
  auto const& cameraAgentTransform = AgentState::get<BodyState>()->getCameraAgentTransform();

  int width = d_cameraModel->imageWidth();
  int height = d_cameraModel->imageHeight();

  int horiz1 = min(height - 1, findHorizonForColumn(0, cameraAgentTransform));
  int horiz2 = min(height - 1, findHorizonForColumn(width - 1, cameraAgentTransform));

  auto const& p1 = findGroundPointForPixel(Vector2d(0, 0) + Vector2d(0.5,0.5));
  if (p1)
    visibleFieldPoly.push_back(*p1);

  if (horiz1 >= 0 && horiz1 < height)
  {
    auto const& p3 = findGroundPointForPixel(Vector2d(0, horiz1) + Vector2d(0.5,0.5));
    if (p3)
      visibleFieldPoly.push_back(*p3);
  }

  if (horiz2 >= 0 && horiz2 < height)
  {
    auto const& p4 = findGroundPointForPixel(Vector2d(width - 1, horiz2) + Vector2d(0.5,0.5));
    if (p4)
      visibleFieldPoly.push_back(*p4);
  }

  auto const& p2 = findGroundPointForPixel(Vector2d(width - 1, 0) + Vector2d(0.5,0.5));
  if (p2)
    visibleFieldPoly.push_back(*p2);

  AgentState::getInstance().set(make_shared<AgentFrameState const>(ball, goals, lineSegments, visibleFieldPoly));
}
