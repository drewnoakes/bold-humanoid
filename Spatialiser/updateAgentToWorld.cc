#include "spatialiser.hh"

#include "../State/state.hh"
#include "../StateObject/BodyState/bodystate.hh"
#include "../StateObject/AgentFrameState/agentframestate.hh"
#include "../StateObject/WorldFrameState/worldframestate.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

void Spatialiser::updateAgentToWorld(AgentPosition position)
{
  auto agentFrame = State::get<AgentFrameState>();

  //
  // Transform from agent to world space
  //

  Affine3d worldAgentTransform = position.worldAgentTransform();

  // Project ball observation
  auto const& ballAgent = agentFrame->getBallObservation();
  Maybe<Vector3d> ball = ballAgent.hasValue()
    ? worldAgentTransform * (*ballAgent)
    : Maybe<Vector3d>::empty();

  // Project goal observations
  vector<Vector3d> goals;
  for (auto const& goalPos : agentFrame->getGoalObservations())
  {
    goals.emplace_back(worldAgentTransform * goalPos);
  }

  // Project observed lines
  vector<LineSegment3d> lineSegments;
  for (auto const& lineSegmentAgent : agentFrame->getObservedLineSegments())
  {
    auto lineSegment = LineSegment3d(
      worldAgentTransform * lineSegmentAgent.p1(),
      worldAgentTransform * lineSegmentAgent.p2()
    );
    lineSegments.emplace_back(lineSegment);
  }

  // Project occlusion rays
  vector<OcclusionRay<double>> occlusionRays;
  // TODO perform the rotation/translation in 2D (where z == 0), not in 3D
  for (auto const& ray : agentFrame->getOcclusionRays())
  {
    Vector3d near = ray.near().head<3>();
    Vector3d far = ray.far().head<3>();

    occlusionRays.emplace_back((worldAgentTransform * near).head<2>(),
                               (worldAgentTransform * far).head<2>());
  }

  // Determine observed field area polygon
  Polygon2d::PointVector vertices;
  if (agentFrame->getVisibleFieldPoly().hasValue())
  {
    for (Vector2d const& vertex : agentFrame->getVisibleFieldPoly().value())
    {
      Vector3d vertex3;
      vertex3 << vertex.x(), vertex.y(), 0;
      vertices.emplace_back((worldAgentTransform * vertex3).head<2>());
    }
  }
  Maybe<Polygon2d> visibleFieldPoly = vertices.size() == 4 ? Maybe<Polygon2d>(Polygon2d(vertices)) : Maybe<Polygon2d>::empty();

  State::make<WorldFrameState>(ball, goals, lineSegments, visibleFieldPoly, occlusionRays, position);
}
