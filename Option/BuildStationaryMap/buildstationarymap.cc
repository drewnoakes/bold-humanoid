#include "buildstationarymap.hh"

#include "../../State/state.hh"
#include "../../StateObject/AgentFrameState/agentframestate.hh"
#include "../../StateObject/StationaryMapState/stationarymapstate.hh"

using namespace bold;
using namespace Eigen;
using namespace rapidjson;
using namespace std;

// TODO decrease the 'score' of an estimate if it should be seen, but isn't
// TODO build a map of the closest occluded area, radially from our position -- prove by side-kicking instead of forward kicking when keeper in the way

BuildStationaryMap::BuildStationaryMap(string const& id)
: Option(id, "BuildStationaryMap")
{}

vector<shared_ptr<Option>> BuildStationaryMap::runPolicy(Writer<StringBuffer>& writer)
{
  auto agentFrame = State::get<AgentFrameState>();

  ASSERT(agentFrame);

  bool hasChange = false;

  if (agentFrame->isBallVisible())
  {
    integrate(d_ballEstimates, agentFrame->getBallObservation().value(), 0.2); // TODO magic number!
    hasChange = true;
  }

  for (auto const& goal : agentFrame->getGoalObservations())
  {
    integrate(d_goalEstimates, goal, 0.5); // TODO magic number!
    hasChange = true;
  }

  for (auto const& teammate : agentFrame->getTeamMateObservations())
  {
    integrate(d_teammateEstimates, teammate, 0.5); // TODO magic number!
    hasChange = true;
  }

  if (hasChange)
    updateStateObject();

  return {};
}

void BuildStationaryMap::updateStateObject() const
{
  State::make<StationaryMapState>(d_ballEstimates, d_goalEstimates, d_teammateEstimates);
}

void BuildStationaryMap::reset()
{
  d_ballEstimates.clear();
  d_goalEstimates.clear();
  d_teammateEstimates.clear();

  updateStateObject();
}

void BuildStationaryMap::integrate(vector<Average<Vector3d>>& estimates, Vector3d pos, double mergeDistance)
{
  for (auto& estimate : estimates)
  {
    double dist = (estimate.getAverage() - pos).norm();
    if (dist <= mergeDistance)
    {
      // Merge into existing estimate and return
      estimate.add(pos);
      return;
    }
  }

  // No estimate matched, so create a new one
  estimates.emplace_back();
  estimates[estimates.size() - 1].add(pos);
}
