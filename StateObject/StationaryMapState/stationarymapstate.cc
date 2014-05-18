#include "stationarymapstate.hh"

#include "../../Config/config.hh"
#include "../../FieldMap/fieldmap.hh"
#include "../../Option/KickOption/kickoption.hh"
#include "../../State/state.hh"
#include "../../StateObject/TeamState/teamstate.hh"

using namespace bold;
using namespace Eigen;
using namespace rapidjson;
using namespace std;

StationaryMapState::StationaryMapState(
  std::vector<Average<Eigen::Vector3d>> ballEstimates,
  std::vector<Average<Eigen::Vector3d>> goalEstimates,
  std::vector<Average<Eigen::Vector3d>> teammateEstimates)
: d_ballEstimates(ballEstimates),
  d_teammateEstimates(teammateEstimates)
{
  // Sort estimates such that those with greater numbers of observations appear first
  std::sort(d_ballEstimates.begin(), d_ballEstimates.end(), compareAverages);
  std::sort(d_teammateEstimates.begin(), d_teammateEstimates.end(), compareAverages);
  std::sort(goalEstimates.begin(), goalEstimates.end(), compareAverages);

  d_goalEstimates = labelGoalObservations(d_teammateEstimates, goalEstimates);
}

void StationaryMapState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("balls").StartArray();
    for (auto const& estimate : d_ballEstimates)
    {
      writer.StartObject();
      {
        writer.String("pos").StartArray().Double(estimate.getAverage().x()).Double(estimate.getAverage().y()).EndArray();
        writer.String("count").Int(estimate.getCount());
      }
      writer.EndObject();
    }
    writer.EndArray();

    writer.String("goals").StartArray();
    for (auto const& estimate : d_goalEstimates)
    {
      writer.StartObject();
      {
        writer.String("pos").StartArray().Double(estimate.getAverage().x()).Double(estimate.getAverage().y()).EndArray();
        writer.String("count").Int(estimate.getCount());
        writer.String("label").String(getGoalLabelName(estimate.getLabel()).c_str());
      }
      writer.EndObject();
    }
    writer.EndArray();

    writer.String("teammates").StartArray();
    for (auto const& estimate : d_teammateEstimates)
    {
      writer.StartObject();
      {
        writer.String("pos").StartArray().Double(estimate.getAverage().x()).Double(estimate.getAverage().y()).EndArray();
        writer.String("count").Int(estimate.getCount());
      }
      writer.EndObject();
    }
    writer.EndArray();
  }
  writer.EndObject();
}

uint StationaryMapState::countGoalsWithSamples(int sampleThreshold) const
{
  uint count = 0;
  for (auto const& goal : d_goalEstimates)
  {
    if (goal.getCount() >= sampleThreshold)
      count++;
    else
      break; // list is ordered
  }
  return count;
}

bool StationaryMapState::hasBallWithSamples(int sampleThreshold) const
{
  return d_ballEstimates.size() && d_ballEstimates[0].getCount() >= sampleThreshold;
}

vector<GoalEstimate> StationaryMapState::labelGoalObservations(
  vector<Average<Vector3d>> const& keeperEstimates,
  vector<Average<Vector3d>> const& goalEstimates)
{
  static auto maxGoalieGoalDistance = Config::getSetting<double>("vision.player-detection.max-goalie-goal-dist");
  static auto maxGoalPairDistanceError = Config::getSetting<double>("vision.goal-detection.max-pair-error-dist");
  const double maxPositionMeasurementError = 0.4; // TODO review this experimentally and move to config
  double theirsThreshold = Vector2d(
    (FieldMap::fieldLengthX() / 2.0) + maxPositionMeasurementError,
     FieldMap::fieldLengthY() / 2.0
  ).norm();

  // Use what we see to try and label goal posts as belonging to either our
  // team, their team, or having unknown association.
  //
  // For example, if the keeper tells us that the ball is 1m from them, and
  // we observe the ball as 1.5m from the goal post, that post belongs to us.
  //
  // Also, if two posts span roughly the expected width of a goal and we see
  // a keeper between them, this is our goal.

  auto team = State::get<TeamState>();

  FieldSide ballSide = team ? team->getKeeperBallSideEstimate() : FieldSide::Unknown;

  auto getLabel = [&ballSide,&goalEstimates,&keeperEstimates,maxPositionMeasurementError,theirsThreshold](Vector3d goalEstimate) -> GoalLabel
  {
    switch (ballSide)
    {
      // Our keeper doesn't know where the ball is, so base purely upon whether
      // we see our keeper between the goal posts.
      case FieldSide::Unknown:
      {
        // The ball won't break the symmetry.
        // Try to look for the keeper in the image.
        // See if this goal estimate has a viable partner, then test for a keeper
        bool foundPair = false;
        for (auto const& otherGoal : goalEstimates)
        {
          // Stop looping when the estimates are not confident enough
          if (otherGoal.getCount() < KickOption::GoalSamplesNeeded)
            break;
          // Skip the one being processed -- we only want pairs
          if (otherGoal.getAverage() == goalEstimate)
            continue;
          // Measure the distance between the posts
          double space = (otherGoal.getAverage() - goalEstimate).head<2>().norm();
          // Verify the distance is what we expect, within a specified error distance
          if (fabs(space - FieldMap::getGoalY()) > maxGoalPairDistanceError->getValue())
            continue;
          // Check if we see goalie in between this pair of posts
          auto mid = (otherGoal.getAverage() + goalEstimate) / 2.0;
          for (auto const& keeperEstimate : keeperEstimates)
          {
            // Stop looping when the estimates are not confident enough
            if (keeperEstimate.getCount() < KickOption::KeeperSamplesNeeded)
              break;
            if ((mid - keeperEstimate.getAverage()).head<2>().norm() < maxGoalieGoalDistance->getValue())
            {
              log::info("BuildStationaryMap::labelGoalObservations") << "Observe keeper between goal posts, so assume as ours";
              return GoalLabel::Ours;
            }
          }
          foundPair = true;
        }
        // If we found a good goal pair, but no keeper between, it's theirs.
        return foundPair ? GoalLabel::Theirs : GoalLabel::Unknown;
      }
      // We have an estimate of the keeper's distance from the ball
      //
      // ASSUME the ball is approximately at our feet
      case FieldSide::Ours:
      {
        double goalDist = goalEstimate.norm();
        if (goalDist < (FieldMap::fieldLengthX()/2.0) - maxPositionMeasurementError)
        {
          log::info("BuildStationaryMap::labelGoalObservations") << "Keeper believes ball is on our side, and closest goal is too close at " << goalDist;
          return GoalLabel::Ours;
        }
        break;
      }
      case FieldSide::Theirs:
      {
        double goalDist = goalEstimate.norm();
        if (goalDist > theirsThreshold)
        {
          log::info("BuildStationaryMap::labelGoalObservations") << "Keeper believes ball is on the opponent's side, and closest goal is too far at " << goalDist;
          return GoalLabel::Ours;
        }
        break;
      }
    }

    return GoalLabel::Unknown;
  };

  // Build labelled goal vector
  vector<GoalEstimate> labelledEstimates;
  labelledEstimates.reserve(goalEstimates.size());
  std::transform(goalEstimates.begin(), goalEstimates.end(),
                 labelledEstimates.begin(),
                 [&getLabel](Average<Vector3d> const& goalEstimate)
                 {
                   auto label = getLabel(goalEstimate.getAverage());
                   return GoalEstimate(goalEstimate, label);
                });
  return labelledEstimates;
}

string bold::getGoalLabelName(GoalLabel label)
{
  switch (label)
  {
    case GoalLabel::Ours:    return "Ours";
    case GoalLabel::Theirs:  return "Theirs";
    case GoalLabel::Unknown: return "Unknown";
  }
  ASSERT(false);
  return "Unexpected enum value.";
}
