#include "stationarymapstate.hh"

#include "../../Config/config.hh"
#include "../../FieldMap/fieldmap.hh"
#include "../../State/state.hh"
#include "../../StateObject/TeamState/teamstate.hh"

#include <deque>
#include <vector>

using namespace bold;
using namespace Eigen;
using namespace rapidjson;
using namespace std;

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

///////////////////////////////////////////////////////////////////////////////

StationaryMapState::StationaryMapState(
  std::vector<Average<Eigen::Vector3d>> ballEstimates,
  std::vector<Average<Eigen::Vector3d>> goalPostEstimates,
  std::vector<Average<Eigen::Vector3d>> keeperEstimates)
: d_ballEstimates(ballEstimates),
  d_keeperEstimates(keeperEstimates)
{
  // Sort estimates such that those with greater numbers of observations appear first
  std::sort(d_ballEstimates.begin(), d_ballEstimates.end(), compareAverages);
  std::sort(d_keeperEstimates.begin(), d_keeperEstimates.end(), compareAverages);
  std::sort(goalPostEstimates.begin(), goalPostEstimates.end(), compareAverages);

  d_goalPostEstimates = labelGoalPostObservations(d_keeperEstimates, goalPostEstimates);

  findGoals();

  selectKick();

  // TODO include the desired agent-frame ball pos at the end of the turn
  calculateTurnAngle();
}

vector<GoalPostEstimate> StationaryMapState::labelGoalPostObservations(
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
          if (otherGoal.getCount() < GoalSamplesNeeded)
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
            if (keeperEstimate.getCount() < KeeperSamplesNeeded)
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
  vector<GoalPostEstimate> labelledEstimates;
  labelledEstimates.resize(goalEstimates.size());
  std::transform(goalEstimates.begin(), goalEstimates.end(),
                 labelledEstimates.begin(),
                 [&getLabel](Average<Vector3d> const& goalEstimate)
                 {
                   auto label = getLabel(goalEstimate.getAverage());
                   return GoalPostEstimate(goalEstimate, label);
                });
  return labelledEstimates;
}

void StationaryMapState::calculateTurnAngle()
{
  if (d_selectedKick != nullptr || d_ballEstimates.size() == 0 || d_goalEstimates.size() == 0)
  {
    d_turnAngleRads = 0.0;
    d_turnBallPos = Vector2d::Zero();
    return;
  }

  // We see the ball and one goal.
  // Find the shortest turn required to make a good kick.
  // If the wrong goal, compute the desired angle(s) away from it.
  // Ask each kick what it's ideal ball position is. Set this on CircleBall.

  // Find the target angle(s) that would be good from here.

  // TODO don't assume the goal is theirs!
  // TODO don't assume we only see one goal!
  // TODO consider obstacles in the goal -- don't just aim for the middle
  // TODO handle the case where we only see one goal post and it's labelled as belonging to the opponent
  // TODO some decisions about turning should only be made once the entire area has been surveyed -- no way to express this currently

  if (d_goalEstimates[0].getLabel() == GoalLabel::Ours)
  {
    d_turnAngleRads = M_PI;
    d_turnBallPos = Vector2d();
    return;
  }

  vector<Vector3d> targetPositions = {
    d_goalEstimates[0].getMidpoint(0.25),
    d_goalEstimates[0].getMidpoint(0.5),
    d_goalEstimates[0].getMidpoint(0.25)
  };

  vector<double> targetAngles;
  targetAngles.reserve(targetPositions.size());
  std::transform(targetPositions.begin(), targetPositions.end(),
                 back_inserter(targetAngles),
                 [](Vector3d const& target) { return atan2(-target.x(), target.y()); });

  double closestAngle = std::numeric_limits<double>::max();
  Vector2d closestBallPos = {};
  bool found = false;

  for (shared_ptr<Kick const> const& kick : Kick::getAll())
  {
    Vector2d ballPos = kick->getIdealBallPos();
    Maybe<Vector2d> endPos = kick->estimateEndPos(ballPos);
    ASSERT(endPos.hasValue());
    double angle = atan2(-endPos->x(), endPos->y());
    for (double const& targetAngle : targetAngles)
    {
      if (fabs(closestAngle) > fabs(angle - targetAngle))
      {
        closestAngle = angle - targetAngle;
        closestBallPos = ballPos;
        found = true;
      }
    }
  }

  if (found)
  {
    d_turnAngleRads = -closestAngle;
    d_turnBallPos = closestBallPos;
  }
}

void StationaryMapState::findGoals()
{
  std::deque<GoalPostEstimate> posts(d_goalPostEstimates.begin(), d_goalPostEstimates.end());

  while (posts.size() > 1)
  {
    auto post1 = posts.front();

    if (post1.getCount() < GoalSamplesNeeded)
      break;

    posts.pop_front();

    for (auto post2 = posts.begin(); post2 != posts.end(); post2++)
    {
      if (post2->getCount() < GoalSamplesNeeded)
        break;

      double dist = (post1.getAverage() - post2->getAverage()).norm();
      double error = fabs(FieldMap::getGoalY() - dist);

      // TODO error should be a function of distance, as uncertainty increases

      if (error < 0.5) // TODO magic number
      {
        GoalLabel label;
        if (post1.getLabel() == GoalLabel::Theirs || post2->getLabel() == GoalLabel::Theirs)
          label = GoalLabel::Theirs;
        else if (post1.getLabel() == GoalLabel::Ours || post2->getLabel() == GoalLabel::Ours)
          label = GoalLabel::Ours;
        else
          label = GoalLabel::Unknown;

        d_goalEstimates.emplace_back(post1.getAverage(), post2->getAverage(), label);

        posts.erase(post2);
        break;
      }
    }
  }
}

void StationaryMapState::selectKick()
{
  // Short circuit if we don't have enough observations
  if (d_ballEstimates.size() == 0 || d_goalPostEstimates.size() < 2)
    return;

  auto const& ballEstimate = d_ballEstimates[0];

  if (ballEstimate.getCount() < BallSamplesNeeded)
    return;

  for (auto& kick : Kick::getAll())
  {
    Maybe<Vector2d> endPos = kick->estimateEndPos(ballEstimate.getAverage().head<2>());

    // If no end position, then this kick is not possible given the ball's
    // current position.
    if (!endPos.hasValue())
      continue;

    double endAngle = atan2(endPos->x(), endPos->y());

    // Determine whether the end pos is advantageous
    bool hasLeft = false, hasRight = false;
    for (auto const& goal : d_goalPostEstimates)
    {
      if (goal.getCount() < GoalSamplesNeeded)
        break;

      auto goalPos = goal.getAverage();
      double goalAngle = atan2(goalPos.x(), goalPos.y());

      if (goalAngle > endAngle)
        hasRight = true;
      else
        hasLeft = true;
    }

    bool isOnTarget = hasLeft && hasRight;
    d_possibleKicks.emplace_back(kick, endPos.value(), isOnTarget);
  }

  // TODO when more than one kick is possible, take the best, not the first
  // TODO the end pos doesn't necessarily have to be between the goals -- sometimes just nearer the goal is enough
  auto it = find_if(d_possibleKicks.begin(), d_possibleKicks.end(), [](KickResult const& k) { return k.isOnTarget(); });
  if (it != d_possibleKicks.end())
    d_selectedKick =  it->getKick();
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

    writer.String("goalPosts").StartArray();
    for (auto const& estimate : d_goalPostEstimates)
    {
      writer.StartObject();
      {
        writer.String("pos").StartArray().Double(estimate.getAverage().x()).Double(estimate.getAverage().y()).EndArray();
        writer.String("count").Uint(estimate.getCount());
        writer.String("label").Uint(static_cast<int>(estimate.getLabel()));
      }
      writer.EndObject();
    }
    writer.EndArray();

    writer.String("goals").StartArray();
    for (auto const& estimate : d_goalEstimates)
    {
      writer.StartObject();
      {
        writer.String("post1").StartArray().Double(estimate.getPost1().x()).Double(estimate.getPost1().y()).EndArray();
        writer.String("post2").StartArray().Double(estimate.getPost2().x()).Double(estimate.getPost2().y()).EndArray();
        writer.String("label").Uint(static_cast<int>(estimate.getLabel()));
      }
      writer.EndObject();
    }
    writer.EndArray();

    writer.String("keepers").StartArray();
    for (auto const& estimate : d_keeperEstimates)
    {
      writer.StartObject();
      {
        writer.String("pos").StartArray().Double(estimate.getAverage().x()).Double(estimate.getAverage().y()).EndArray();
        writer.String("count").Int(estimate.getCount());
      }
      writer.EndObject();
    }
    writer.EndArray();

    writer.String("kicks").StartArray();
    {
      for (auto const& kick : d_possibleKicks)
      {
        writer.StartObject();
        {
          writer.String("id").String(kick.getId().c_str());
          Vector2d const& endPos = kick.getEndPos();
          writer.String("endPos").StartArray().Double(endPos.x()).Double(endPos.y()).EndArray();
          writer.String("onTarget").Bool(kick.isOnTarget());
          writer.String("selected").Bool(kick.getKick() == d_selectedKick);
        }
        writer.EndObject();
      }
    }
    writer.EndArray();

    writer.String("turnAngle").Double(d_turnAngleRads);
    writer.String("turnBallPos").StartArray().Double(d_turnBallPos.x()).Double(d_turnBallPos.y()).EndArray();
  }
  writer.EndObject();
}
