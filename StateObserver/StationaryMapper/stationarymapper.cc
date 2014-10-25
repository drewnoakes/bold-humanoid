#include "stationarymapper.hh"

#include "../../Config/config.hh"
#include "../../StateObject/AgentFrameState/agentframestate.hh"
#include "../../Voice/voice.hh"

using namespace bold;
using namespace Eigen;
using namespace rapidjson;
using namespace std;

// TODO decrease the 'score' of an estimate if it should be seen, but isn't

StationaryMapper::StationaryMapper(std::shared_ptr<Voice> voice)
: TypedStateObserver<WalkState>("Stationary Mapper", ThreadId::ThinkLoop),
  d_hasData(false),
  d_voice(voice)
{}

void StationaryMapper::observeTyped(std::shared_ptr<WalkState const> const& walkState, SequentialTimer& timer)
{
  if (!walkState->isRunning())
  {
    if (d_hasData)
    {
      d_ballEstimates.clear();
      d_goalEstimates.clear();
      d_teammateEstimates.clear();
      d_occlusionMap.reset();

      updateStateObject();
      d_hasData = false;
    }
    return;
  }

  auto agentFrame = State::get<AgentFrameState>();

  ASSERT(agentFrame);

  bool hasData = false;

  // TODO use agentFrame->shouldSeeAgentFrameGroundPoint(...) to decrease scores

  if (agentFrame->isBallVisible())
  {
    integrate(d_ballEstimates, agentFrame->getBallObservation().value().head<2>(), StationaryMapState::BallMergeDistance);
    hasData = true;
  }

  for (auto const& goal : agentFrame->getGoalObservations())
  {
    integrate(d_goalEstimates, goal.head<2>(), StationaryMapState::GoalPostMergeDistance);
    hasData = true;
  }

  for (auto const& teammate : agentFrame->getTeamMateObservations())
  {
    integrate(d_teammateEstimates, teammate.head<2>(), StationaryMapState::TeammateMergeDistance);
    hasData = true;
  }

  // TODO build map of field edge here, either within occlusion map, or in new model

  // Walk all occlusion rays
  for (auto const& ray : agentFrame->getOcclusionRays())
  {
    if (d_occlusionMap.add(ray))
      hasData = true;
  }

  if (hasData || !State::get<StationaryMapState>())
    updateStateObject();

  d_hasData = hasData;
}

void StationaryMapper::integrate(vector<Average<Vector2d>>& estimates, Vector2d pos, double mergeDistance)
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

void StationaryMapper::updateStateObject() const
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