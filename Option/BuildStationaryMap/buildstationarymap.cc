#include "buildstationarymap.hh"

#include "../../State/state.hh"
#include "../../StateObject/AgentFrameState/agentframestate.hh"

using namespace bold;
using namespace Eigen;
using namespace rapidjson;
using namespace std;

#include "../../Voice/voice.hh"
#include "../../Config/config.hh"

// TODO decrease the 'score' of an estimate if it should be seen, but isn't

BuildStationaryMap::BuildStationaryMap(string const& id, std::shared_ptr<Voice> voice)
: Option(id, "BuildStationaryMap"),
  d_voice(voice)
{}

vector<shared_ptr<Option>> BuildStationaryMap::runPolicy(Writer<StringBuffer>& writer)
{
  auto agentFrame = State::get<AgentFrameState>();

  ASSERT(agentFrame);

  bool hasChange = false;

  // TODO use agentFrame->shouldSeeAgentFrameGroundPoint(...) to decrease scores

  if (agentFrame->isBallVisible())
  {
    integrate(d_ballEstimates, agentFrame->getBallObservation().value(), StationaryMapState::BallMergeDistance);
    hasChange = true;
  }

  for (auto const& goal : agentFrame->getGoalObservations())
  {
    integrate(d_goalEstimates, goal, StationaryMapState::GoalPostMergeDistance);
    hasChange = true;
  }

  for (auto const& teammate : agentFrame->getTeamMateObservations())
  {
    integrate(d_teammateEstimates, teammate, StationaryMapState::TeammateMergeDistance);
    hasChange = true;
  }

  // Walk all occlusion rays
  for (auto const& ray : agentFrame->getOcclusionRays())
  {
    if (d_occlusionMap.add(ray))
      hasChange = true;
  }

  if (hasChange || !State::get<StationaryMapState>())
    updateStateObject();

  writer.String("ball").Bool(agentFrame->isBallVisible());
  writer.String("goals").Uint(agentFrame->getGoalObservations().size());
  writer.String("keepers").Uint(agentFrame->getTeamMateObservations().size());
  writer.String("change").Bool(hasChange);

  return {};
}

void BuildStationaryMap::updateStateObject() const
{
  auto map = State::make<StationaryMapState>(d_ballEstimates, d_goalEstimates, d_teammateEstimates, d_occlusionMap);

  static auto announceTurn = Config::getSetting<bool>("options.announce-stationary-map-action");

  if (announceTurn->getValue() && map->getTurnAngleRads() != 0 && d_voice->queueLength() < 2)
  {
    stringstream msg;
    int degrees = (int)Math::radToDeg(map->getTurnAngleRads());
    msg << "Turning " << abs(degrees) << " degree" << (abs(degrees) == 1 ? "" : "s")
        << (degrees < 0 ? " right" : degrees > 0 ? " left" : "")
        << " to kick " << map->getTurnForKick()->getId();
    d_voice->say(msg.str());
  }
}

void BuildStationaryMap::reset()
{
  d_ballEstimates.clear();
  d_goalEstimates.clear();
  d_teammateEstimates.clear();
  d_occlusionMap.reset();

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
