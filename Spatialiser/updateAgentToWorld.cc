#include "spatialiser.hh"

#include "../AgentState/agentstate.hh"
#include "../StateObject/BodyState/bodystate.hh"
#include "../StateObject/AgentFrameState/agentframestate.hh"
#include "../StateObject/WorldFrameState/worldframestate.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

void Spatialiser::updateAgentToWorld()
{
  auto body = AgentState::getInstance().get<BodyState>();
  auto agentFrame = AgentState::getInstance().get<AgentFrameState>();

  auto const& ball = agentFrame->getBallObservation();
  auto const& goals = agentFrame->getGoalObservations();
  auto const& lineSegments = agentFrame->getObservedLineSegments();

  // TODO use the localiser to project from agent to world space

  AgentState::getInstance().set(make_shared<WorldFrameState>(ball, goals, lineSegments));
}
