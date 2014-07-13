#include "spatialiser.ih"

#include "../FieldMap/fieldmap.hh"

void Spatialiser::updateCameraToAgent()
{
  auto cameraFrame = State::get<CameraFrameState>();

  static double ballRadius = FieldMap::getBallDiameter() / 2.0;
  double goalieMarkerHeight = Config::getValue<double>("vision.player-detection.goalie-marker-height");

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

  // Project team mate observations
  std::vector<Vector3d> teamMates;
  for (auto const& teamMate : cameraFrame->getTeamMateObservations())
  {
    auto const& pos3d = findGroundPointForPixel(teamMate, goalieMarkerHeight);
    if (pos3d.hasValue())
      teamMates.emplace_back(*pos3d);
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

  // Find line junctions
  auto lineJunctions = d_lineJunctionFinder->findLineJunctions(lineSegments);

  // Project occlusion rays
  vector<OcclusionRay<double>> occlusionRays;
  for (auto const& ray : cameraFrame->getOcclusionRays())
  {
    auto const& p1 = findGroundPointForPixel(ray.near().cast<double>() + Vector2d(0.5,0.5));
    auto const& p2 = findGroundPointForPixel(ray.far().cast<double>() + Vector2d(0.5,0.5));
    if (p1.hasValue() && p2.hasValue())
      occlusionRays.emplace_back(p1->head<2>(), p2->head<2>());
  }

  // Determine observed field area polygon
  Polygon2d::PointVector vertices;

  static int width = d_cameraModel->imageWidth();
  static int height = d_cameraModel->imageHeight();

  int horiz1 = min(height - 1, findHorizonForColumn(0));
  int horiz2 = min(height - 1, findHorizonForColumn(width - 1));

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

  State::make<AgentFrameState>(ball, goals, teamMates,
                               lineSegments, lineJunctions,
                               visibleFieldPoly, occlusionRays,
                               cameraFrame->getThinkCycleNumber());
}
