#include "spatialiser.ih"

#include "../geometry/Polygon2.hh"

void Spatialiser::updateCameraToAgent()
{
  d_zeroGroundPixelTr = findGroundPixelTransform(0.0);

  auto cameraFrame = State::get<CameraFrameState>();

  static double ballRadius = Config::getStaticValue<double>("world.ball-diameter") / 2.0;

  // Project ball observation
  auto const& ballObs = cameraFrame->getBallObservation();
  Maybe<Vector3d> ball = ballObs.hasValue()
    ? findGroundPointForPixel(*ballObs, ballRadius)
    : Maybe<Vector3d>::empty();

  // Project goal observations
  std::vector<Vector3d> goals;
  for (Vector2d const& goal : cameraFrame->getGoalObservations())
  {
    auto const& pos3d = findGroundPointForPixel(goal);
    if (pos3d.hasValue())
      goals.emplace_back(*pos3d);
  }

  // Project observed lines
  std::vector<LineSegment3d> lineSegments;
  for (LineSegment2i const& lineSegment : cameraFrame->getObservedLineSegments())
  {
    auto const& p1 = findGroundPointForPixel(lineSegment.p1().cast<double>() + Vector2d(0.5,0.5));
    auto const& p2 = findGroundPointForPixel(lineSegment.p2().cast<double>() + Vector2d(0.5,0.5));
    if (p1.hasValue() && p2.hasValue())
      lineSegments.emplace_back(*p1, *p2);
  }

  // Project occlusion rays
  vector<pair<Vector3d,Vector3d>> occlusionRays;
  for (pair<Vector2i,Vector2i> const& pair : cameraFrame->getOcclusionRays())
  {
    auto const& p1 = findGroundPointForPixel(pair.first.cast<double>() + Vector2d(0.5,0.5));
    auto const& p2 = findGroundPointForPixel(pair.second.cast<double>() + Vector2d(0.5,0.5));
    if (p1.hasValue() && p2.hasValue())
      occlusionRays.emplace_back(*p1, *p2);
  }

  // Determine observed field area polygon
  Polygon2d::PointVector vertices;
  auto const& agentCameraTransform = State::get<BodyState>(StateTime::CameraImage)->getAgentCameraTransform();

  static int width = d_cameraModel->imageWidth();
  static int height = d_cameraModel->imageHeight();

  int horiz1 = min(height - 1, findHorizonForColumn(0, agentCameraTransform));
  int horiz2 = min(height - 1, findHorizonForColumn(width - 1, agentCameraTransform));

  auto const& p1 = findGroundPointForPixel(Vector2d(0, 0) + Vector2d(0.5,0.5));
  if (p1)
    vertices.emplace_back(p1->head<2>());

  auto const& p2 = findGroundPointForPixel(Vector2d(width - 3, 0) + Vector2d(0.5,0.5));
  if (p2)
    vertices.emplace_back(p2->head<2>());

  if (horiz2 >= 0 && horiz2 < height)
  {
    auto const& p3 = findGroundPointForPixel(Vector2d(width - 3, horiz2) + Vector2d(0.5,0.5));
    if (p3)
      vertices.emplace_back(p3->head<2>());
  }

  if (horiz1 >= 0 && horiz1 < height)
  {
    auto const& p4 = findGroundPointForPixel(Vector2d(0, horiz1) + Vector2d(0.5,0.5));
    if (p4)
      vertices.emplace_back(p4->head<2>());
  }

  Maybe<Polygon2d> visibleFieldPoly = vertices.size() == 4 ? Maybe<Polygon2d>(Polygon2d(vertices)) : Maybe<Polygon2d>::empty();

  State::set(make_shared<AgentFrameState const>(ball, goals, lineSegments, visibleFieldPoly, occlusionRays));
}
