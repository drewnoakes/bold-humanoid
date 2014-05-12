#include "kickoption.hh"

#include "../../Agent/agent.hh"
#include "../../StateObject/StationaryMapState/stationarymapstate.hh"
#include "../../StateObserver/FallDetector/falldetector.hh"
#include "../MotionScriptOption/motionscriptoption.hh"

using namespace bold;
using namespace Eigen;
using namespace rapidjson;
using namespace std;

Kick::Kick(string id, string scriptPath, Bounds2d ballBounds, Vector2d endPos)
: d_id(id),
  d_scriptPath(scriptPath),
  d_ballBounds(ballBounds),
  d_endPos(endPos)
{}

Maybe<Vector2d> Kick::estimateEndPos(Vector2d const& ballPos) const
{
  // TODO use a more realistic model -- the end pos actually depends upon the starting position -- use kick-learner to build data on this

  if (!d_ballBounds.contains(ballPos))
    return Maybe<Vector2d>::empty();

  return Maybe<Vector2d>(d_endPos);
}

shared_ptr<MotionScriptOption> Kick::createScriptOption(shared_ptr<MotionScriptModule> motionScriptModule) const
{
  return make_shared<MotionScriptOption>(d_id, motionScriptModule, d_scriptPath);
}


//////////////////////////////////////////////////////////////////////////////

KickOption::KickOption(const string& id, Agent* agent)
: Option(id, "Kick"),
  d_agent(agent),
  d_hasRun(false)
{
  // TODO the parameters for these kicks must be determined experimentally

  d_kicks.emplace_back(
    "forward-left",
    "./motionscripts/kick-left.json",
    Bounds2d(Vector2d(0, 0), Vector2d(0.1, 0.2)),
    Vector2d(0, 1.5)
  );

  d_kicks.emplace_back(
    "forward-right",
    "./motionscripts/kick-right.json",
    Bounds2d(Vector2d(-0.1, 0), Vector2d(0, 0.2)),
    Vector2d(0, 1.5)
  );

  d_kicks.emplace_back(
    "side-left",
    "./motionscripts/kick-side-left.json",
    Bounds2d(Vector2d(-0.1, 0), Vector2d(0.05, 0.2)),
    Vector2d(1.5, 1.5)
  );

  d_kicks.emplace_back(
    "side-right",
    "./motionscripts/kick-side-right.json",
    Bounds2d(Vector2d(-0.05, 0), Vector2d(0.1, 0.2)),
    Vector2d(-1.5, 1.5)
  );
}

bool KickOption::canKick() const
{
  return selectKick() != nullptr;
}

Kick const* KickOption::selectKick() const
{
  const int samplesNeeded = 10; // TODO magic number!!

  auto map = State::get<StationaryMapState>();

  if (!map)
    return nullptr;

  if (map->getBallEstimates().size() == 0)
    return nullptr;
  if (map->getGoalEstimates().size() < 2)
    return nullptr;
  if (map->getGoalEstimates()[1].getCount() < samplesNeeded)
    return nullptr;

  auto const& ballEstimate = map->getBallEstimates()[0];

  if (ballEstimate.getCount() < samplesNeeded)
    return nullptr;

  // TODO when more than one kick is possible, take the best, not the first
  // TODO the end pos doens't necessarily have to be between the goals -- sometimes just nearer the goal is enough

  for (auto const& kick : d_kicks)
  {
    auto endPos = kick.estimateEndPos(ballEstimate.getAverage().head<2>());

    if (!endPos.hasValue())
      continue;

    double endAngle = atan2(endPos->x(), endPos->y());

    // Determine whether the end pos is advantageous
    bool hasLeft = false, hasRight = false;
    for (auto const& goal : map->getGoalEstimates())
    {
      if (goal.getCount() < samplesNeeded)
        break;

      auto goalPos = goal.getAverage();
      double goalAngle = atan2(goalPos.x(), goalPos.y());

      if (goalAngle > endAngle)
        hasRight = true;
      else
        hasLeft = true;
    }

    if (hasLeft && hasRight)
      return &kick;
  }

  return nullptr;
}

vector<shared_ptr<Option>> KickOption::runPolicy(Writer<StringBuffer>& writer)
{
  if (!d_hasRun)
  {
    auto kick = selectKick();
    if (kick != nullptr)
      d_activeScript = kick->createScriptOption(d_agent->getMotionScriptModule());
    d_hasRun = true;
  }

  if (d_activeScript != nullptr)
    return {d_activeScript};

  return {};
}

void KickOption::reset()
{
  d_activeScript = nullptr;
  d_hasRun = false;
}

double KickOption::hasTerminated()
{
  return d_hasRun && (!d_activeScript || d_activeScript->hasTerminated()) ? 1.0 : 0.0;
}
