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

  auto const& ballAgent = agentFrame->getBallObservation();
  auto const& goalsAgent = agentFrame->getGoalObservations();
  auto const& lineSegmentsAgent = agentFrame->getObservedLineSegments();

  //
  // Transform from agent to world space
  //

  Affine3d agentToWorld = position.agentToWorldTransform();

  Maybe<Vector3d> ball = ballAgent.hasValue()
  ? agentToWorld * *(ballAgent.value())
  : Maybe<Vector3d>::empty();

  vector<Vector3d> goals;
  vector<LineSegment3d> lineSegments;

  for (auto const& goalPos : goalsAgent)
  {
    goals.push_back(agentToWorld * goalPos);
  }

  for (auto const& lineSegmentAgent : lineSegmentsAgent)
  {
    auto lineSegment = LineSegment3d(
      agentToWorld * lineSegmentAgent.p1(),
      agentToWorld * lineSegmentAgent.p2()
    );
    lineSegments.push_back(lineSegment);
  }

  AgentState::getInstance().set(make_shared<WorldFrameState const>(ball, goals, lineSegments, position));
}
