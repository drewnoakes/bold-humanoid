#include "spatialiser.ih"

void Spatialiser::updateCameraToAgent()
{
  auto cameraFrame = AgentState::get<CameraFrameState>();

  auto const& ballObs = cameraFrame->getBallObservation();

  Maybe<Vector3d> ball = ballObs.hasValue()
    ? findGroundPointForPixel(ballObs->cast<int>())
    : Maybe<Vector3d>::empty();

  std::vector<Vector3d> goals;
  std::vector<LineSegment3d> lineSegments;

  for (Vector2f const& goal : cameraFrame->getGoalObservations())
  {
    auto const& pos3d = findGroundPointForPixel(goal.cast<int>());
    if (pos3d.hasValue())
    {
      goals.push_back(*pos3d.value());
    }
  }

  for (LineSegment2i const& lineSegment : cameraFrame->getObservedLineSegments())
  {
    auto const& p1 = findGroundPointForPixel(lineSegment.p1().cast<int>());
    auto const& p2 = findGroundPointForPixel(lineSegment.p2().cast<int>());
    if (p1.hasValue() && p2.hasValue())
    {
      lineSegments.push_back(LineSegment3d(*p1.value(), *p2.value()));
    }
  }

  AgentState::getInstance().set(make_shared<AgentFrameState const>(ball, goals, lineSegments));
}
