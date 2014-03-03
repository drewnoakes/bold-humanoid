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

  Affine3d agentToWorld = position.worldAgentTransform();

  // Project ball observation
  auto const& ballAgent = agentFrame->getBallObservation();
  Maybe<Vector3d> ball = ballAgent.hasValue()
    ? agentToWorld * (*ballAgent)
    : Maybe<Vector3d>::empty();

  // Project goal observations
  vector<Vector3d> goals;
  for (auto const& goalPos : agentFrame->getGoalObservations())
  {
    goals.emplace_back(agentToWorld * goalPos);
  }

  // Project observed lines
  vector<LineSegment3d> lineSegments;
  for (auto const& lineSegmentAgent : agentFrame->getObservedLineSegments())
  {
    auto lineSegment = LineSegment3d(
      agentToWorld * lineSegmentAgent.p1(),
      agentToWorld * lineSegmentAgent.p2()
    );
    lineSegments.emplace_back(lineSegment);
  }

  // Project occlusion rays
  vector<pair<Vector3d,Vector3d>> occlusionRays;
  for (pair<Vector3d,Vector3d> const& pair : agentFrame->getOcclusionRays())
  {
    occlusionRays.emplace_back(agentToWorld * pair.first,
                               agentToWorld * pair.second);
  }

  // Determine observed field area polygon
  Polygon2d::PointVector vertices;
  if (agentFrame->getVisibleFieldPoly().hasValue())
  {
    for (Vector2d const& vertex : agentFrame->getVisibleFieldPoly().value())
    {
      Vector3d vertex3;
      vertex3 << vertex.x(), vertex.y(), 0;
      vertices.emplace_back((agentToWorld * vertex3).head<2>());
    }
  }
  Maybe<Polygon2d> visibleFieldPoly = vertices.size() == 4 ? Maybe<Polygon2d>(Polygon2d(vertices)) : Maybe<Polygon2d>::empty();

  State::set(make_shared<WorldFrameState const>(ball, goals, lineSegments, visibleFieldPoly, occlusionRays, position));
}
