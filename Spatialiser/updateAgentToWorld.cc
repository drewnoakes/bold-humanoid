#include "spatialiser.hh"

#include "../AgentState/agentstate.hh"
#include "../StateObject/BodyState/bodystate.hh"
#include "../StateObject/AgentFrameState/agentframestate.hh"
#include "../StateObject/WorldFrameState/worldframestate.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

void Spatialiser::updateAgentToWorld(AgentPosition position)
{
  auto agentFrame = AgentState::get<AgentFrameState>();

  //
  // Transform from agent to world space
  //

  Affine3d agentToWorld = position.worldAgentTransform();

  auto const& ballAgent = agentFrame->getBallObservation();

  Maybe<Vector3d> ball = ballAgent.hasValue()
    ? agentToWorld * (*ballAgent)
    : Maybe<Vector3d>::empty();

  vector<Vector3d> goals;
  vector<LineSegment3d> lineSegments;
  Polygon2d::PointVector vertices;

  for (auto const& goalPos : agentFrame->getGoalObservations())
  {
    goals.push_back(agentToWorld * goalPos);
  }

  for (auto const& lineSegmentAgent : agentFrame->getObservedLineSegments())
  {
    auto lineSegment = LineSegment3d(
      agentToWorld * lineSegmentAgent.p1(),
      agentToWorld * lineSegmentAgent.p2()
    );
    lineSegments.push_back(lineSegment);
  }

  if (agentFrame->getVisibleFieldPoly().hasValue())
  {
    for (Vector2d const& vertex : agentFrame->getVisibleFieldPoly().value())
    {
      Vector3d vertex3;
      vertex3 << vertex.x(), vertex.y(), 0;
      vertices.push_back((agentToWorld * vertex3).head<2>());
    }
  }
  Maybe<Polygon2d> visibleFieldPoly = vertices.size() == 4 ? Maybe<Polygon2d>(Polygon2d(vertices)) : Maybe<Polygon2d>::empty();

  AgentState::set(make_shared<WorldFrameState const>(ball, goals, lineSegments, visibleFieldPoly, position));
}
