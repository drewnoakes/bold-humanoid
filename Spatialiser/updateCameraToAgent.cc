#include "spatialiser.ih"

void Spatialiser::updateCameraToAgent()
{
  auto cameraFrame = AgentState::get<CameraFrameState>();

  auto const& ballObs = cameraFrame->getBallObservation();

  Maybe<Vector3d> ball = ballObs.hasValue()
    ? findGroundPointForPixel(*ballObs.value())
    : Maybe<Vector3d>::empty();

  std::vector<Vector3d> goals;
  std::vector<LineSegment3d> lineSegments;

  for (Vector2d const& goal : cameraFrame->getGoalObservations())
  {
    auto const& pos3d = findGroundPointForPixel(goal);
    if (pos3d.hasValue())
    {
      goals.push_back(*pos3d.value());
    }
  }

  for (LineSegment2i const& lineSegment : cameraFrame->getObservedLineSegments())
  {
    auto const& p1 = findGroundPointForPixel(lineSegment.p1().cast<double>() + Vector2d(0.5,0.5));
    auto const& p2 = findGroundPointForPixel(lineSegment.p2().cast<double>() + Vector2d(0.5,0.5));
    if (p1.hasValue() && p2.hasValue())
    {
      lineSegments.push_back(LineSegment3d(*p1.value(), *p2.value()));
    }
  }

  AgentState::getInstance().set(make_shared<AgentFrameState const>(ball, goals, lineSegments));
}
