#include "stationarymapstate.hh"

#include "../../Config/config.hh"
#include "../../FieldMap/fieldmap.hh"
#include "../../State/state.hh"
#include "../../StateObject/TeamState/teamstate.hh"

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

bool RadialOcclusionMap::add(OcclusionRay<double> const& ray)
{
  // If the ray is short, then it's probably just noise.
  if (ray.norm() < 0.3) // TODO magic number!
    return false;

  uint index = wedgeIndexForAngle(ray.angle());
  d_wedges[index].add(ray.near().norm());
  return true;
}

void RadialOcclusionMap::reset()
{
  for (auto& wedge : d_wedges)
    wedge.reset();
}

uint RadialOcclusionMap::wedgeIndexForAngle(double angle)
{
  int index = static_cast<int>(round(NumberOfBuckets*angle/(2*M_PI)));
  while (index < 0)
    index += NumberOfBuckets;
  while (index > NumberOfBuckets)
    index -= NumberOfBuckets;
  return static_cast<uint>(index);
}

double RadialOcclusionMap::angleForWedgeIndex(uint index)
{
  return Math::normaliseRads(index*2*M_PI/NumberOfBuckets);
}

double RadialOcclusionMap::getOcclusionDistance(double angle) const
{
  Average<double> const& wedge = d_wedges[wedgeIndexForAngle(angle)];

  if (wedge.getCount() < 3) // TODO magic number!
    return numeric_limits<double>::max();

  return wedge.getAverage();
}

void RadialOcclusionMap::writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("divisions").Uint(NumberOfBuckets);
    writer.String("slices").StartArray();
    {
      for (uint index = 0; index < NumberOfBuckets; index++)
      {
        if (d_wedges[index].getCount() == 0)
          continue;

        writer.StartObject();
        {
          writer.String("angle").Double(angleForWedgeIndex(index));
          writer.String("dist").Double(d_wedges[index].getAverage());
          writer.String("count").Double(d_wedges[index].getCount());
        }
        writer.EndObject();
      }
    }
    writer.EndArray();
  }
  writer.EndObject();
}

///////////////////////////////////////////////////////////////////////////////

GoalEstimate::GoalEstimate(Vector2d const& post1Pos, Vector2d const& post2Pos, GoalLabel post1Label, GoalLabel post2Label)
: d_post1Pos(post1Pos),
  d_post2Pos(post2Pos),
  d_post1Label(post1Label),
  d_post2Label(post2Label)
{
  if (post1Label == GoalLabel::Theirs || post2Label == GoalLabel::Theirs)
    d_label = GoalLabel::Theirs;
  else if (post1Label == GoalLabel::Ours || post2Label == GoalLabel::Ours)
    d_label = GoalLabel::Ours;
  else
    d_label = GoalLabel::Unknown;
}

bool GoalEstimate::isTowards(double ballEndAngle) const
{
  bool hasLeft = false,
       hasRight = false;

  auto testSide = [&](Vector2d const& postPos)
  {
    if (Math::angleToPoint(postPos) > ballEndAngle)
      hasRight = true;
    else
      hasLeft = true;
  };

  testSide(d_post1Pos);
  testSide(d_post2Pos);

  return hasLeft && hasRight;
}

GoalEstimate GoalEstimate::estimateOppositeGoal(GoalLabel label) const
{
  Vector2d goalLine = d_post2Pos - d_post1Pos;

  // There are two perpendicular vectors which should run down the length of the
  // field, parallel to the world's x-axis.
  Vector2d perp1(-goalLine.y(), goalLine.x());
  Vector2d perp2(goalLine.y(), -goalLine.x());

  // Find which one points towards us, as we assume we're between the two goals
  Vector2d perp = perp1.dot(d_post1Pos) < 0
    ? perp1
    : perp2;

  Vector2d fieldX = perp.normalized() * FieldMap::getFieldLengthX();

  return GoalEstimate(d_post1Pos + fieldX, d_post2Pos + fieldX, label, label);
}

///////////////////////////////////////////////////////////////////////////////

StationaryMapState::StationaryMapState(
  std::vector<Average<Vector2d>> ballEstimates,
  std::vector<Average<Vector2d>> goalPostEstimates,
  std::vector<Average<Vector2d>> keeperEstimates,
  RadialOcclusionMap occlusionMap)
: d_ballEstimates(ballEstimates),
  d_keeperEstimates(keeperEstimates),
  d_occlusionMap(occlusionMap)
{
  // Sort estimates such that those with greater numbers of observations appear first
  std::sort(d_ballEstimates.begin(), d_ballEstimates.end(), compareAverages<Vector2d>);
  std::sort(d_keeperEstimates.begin(), d_keeperEstimates.end(), compareAverages<Vector2d>);
  std::sort(goalPostEstimates.begin(), goalPostEstimates.end(), compareAverages<Vector2d>);

  d_goalPostEstimates = labelGoalPostObservations(d_keeperEstimates, goalPostEstimates);

  findGoals();

  // Only attempt to select a kick/turn angle if the ball is within a reasonable distance
  if (hasBallWithinDistance(0.5))
  {
    selectKick();

    calculateTurnAngle();

    static bool errorFlag = false;
    if (hasEnoughBallAndGoalPostObservations())
    {
      if (!d_selectedKick && d_turnAngleRads == 0 && !errorFlag)
      {
        errorFlag = true;
        log::warning("StationaryMapState::StationaryMapState") << "Have enough observations, but no kick or turn selected";
      }
    }
    else
    {
      errorFlag = false;
    }
  }
}

vector<GoalPostEstimate> StationaryMapState::labelGoalPostObservations(
  vector<Average<Vector2d>> const& keeperEstimates,
  vector<Average<Vector2d>> const& goalEstimates)
{
  static auto maxGoalieGoalDistance = Config::getSetting<double>("vision.player-detection.max-goalie-goal-dist");
  static auto maxGoalPairDistanceError = Config::getSetting<double>("vision.goal-detection.max-pair-error-dist");
  const double maxPositionMeasurementError = 0.4; // TODO review this experimentally and move to config
  double theirsThreshold = Vector2d(
    (FieldMap::getFieldLengthX() / 2.0) + maxPositionMeasurementError,
    FieldMap::getFieldLengthY() / 2.0
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

  auto getLabel = [&ballSide,&goalEstimates,&keeperEstimates,maxPositionMeasurementError,theirsThreshold](Vector2d goalEstimate) -> GoalLabel
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
        if (goalDist < (FieldMap::getFieldLengthX()/2.0) - maxPositionMeasurementError)
        {
          log::verbose("BuildStationaryMap::labelGoalObservations") << "Keeper believes ball is on our side, and closest goal is too close at " << goalDist;
          return GoalLabel::Ours;
        }
        break;
      }
      case FieldSide::Theirs:
      {
        double goalDist = goalEstimate.norm();
        if (goalDist > theirsThreshold)
        {
          log::verbose("BuildStationaryMap::labelGoalObservations") << "Keeper believes ball is on the opponent's side, and closest goal is too far at " << goalDist;
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
                 [&getLabel](Average<Vector2d> const& goalEstimate)
                 {
                   auto label = getLabel(goalEstimate.getAverage());
                   return GoalPostEstimate(goalEstimate, label);
                });
  return labelledEstimates;
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

      if (error < GoalLengthErrorDistance)
      {
        d_goalEstimates.emplace_back(post1.getAverage(), post2->getAverage(), post1.getLabel(), post2->getLabel());

        posts.erase(post2);
        break;
      }
    }
  }

  // If we only see our goal, synthesize an estimate of the opposite goal position
  if (d_goalEstimates.size() == 1)
  {
    auto const& goal = d_goalEstimates[0];
    if (goal.getLabel() == GoalLabel::Ours)
      d_goalEstimates.push_back(goal.estimateOppositeGoal(GoalLabel::Theirs));
  }
}

double getGoalLineDistance(GoalEstimate const& goal, Vector2d const& endPos)
{
  // 't' is the ratio of intersection along the line between the goal posts
  // 'u' is the ratio along the length of the kick line

  double t, u;
  goal.lineSegment2d().tryIntersect(LineSegment2d(Vector2d::Zero(), endPos), t, u);

  // Ensure we're not kicking outside the goal
  if (t < 0 || t > 1)
  {
    // NOTE this warning may become invalid later on -- but for our current use case it's correct
    log::warning("getGoalLineDistance") << "Ball end pos is outside goal (t=" << t << ")";
    return numeric_limits<double>::max();
  }

  // Ensure the intersection doesn't occur 'backwards' along the ball's direction of travel
  if (u < 0)
  {
    log::warning("getGoalLineDistance") << "Kick/goal intersection occurs backwards along ball's direction of travel";
    return numeric_limits<double>::max();
  }

  return u * endPos.norm();
}

void StationaryMapState::selectKick()
{
  // Short circuit if we don't have enough observations
  if (d_ballEstimates.size() == 0 || d_goalPostEstimates.size() < 2)
    return;

  auto const& ballEstimate = d_ballEstimates[0];

  if (ballEstimate.getCount() < BallSamplesNeeded)
    return;

  // Check each kick to see if it's possible, and whether it would give a good result
  for (auto& kick : Kick::getAll())
  {
    Maybe<Vector2d> endPos = kick->estimateEndPos(ballEstimate.getAverage().head<2>());

    // If no end position, then this kick is not possible given the ball's current position
    if (!endPos.hasValue())
      continue;

    double ballEndAngle = Math::angleToPoint(*endPos);

    // Determine whether the end pos is advantageous
    bool isOnTarget = false;
    for (auto const& goal : d_goalEstimates)
    {
      // Currently, only kick toward the goal

      // Don't kick towards our goal
      if (goal.getLabel() == GoalLabel::Ours)
        continue;

      // If the kick angle is not directed between the goal posts, skip it
      if (!goal.isTowards(ballEndAngle))
        continue;

      // Check that there's no obstruction before the goal line -- blocks inside the goal are fine
      double occlusionDistance = d_occlusionMap.getOcclusionDistance(ballEndAngle);
      double goalLineDistance = getGoalLineDistance(goal, *endPos);

      // TODO the entire ball must actually cross the line, not just its midpoint
      if (occlusionDistance < goalLineDistance)
        continue;

      log::info("StationaryMapState::selectKick")
        << "Goal kick possible: " << kick->getId()
        << " angleDegs=" << Math::radToDeg(ballEndAngle)
        << " goalLabel=" << getGoalLabelName(goal.getLabel())
        << " post1Label=" << getGoalLabelName(goal.getPost1Label())
        << " post2Label=" << getGoalLabelName(goal.getPost2Label())
        << " occlusionDist=" << occlusionDistance
        << " goalLineDist=" << goalLineDistance;

      isOnTarget = true;
      break;
    }

    d_possibleKicks.emplace_back(kick, endPos.value(), isOnTarget);
  }

  // TODO when more than one kick is possible, take the best, not the first
  // TODO the end pos doesn't necessarily have to be between the goals -- sometimes just nearer the goal is enough
  auto it = find_if(d_possibleKicks.begin(), d_possibleKicks.end(), [](KickResult const& k) { return k.isOnTarget(); });
  if (it != d_possibleKicks.end())
    d_selectedKick =  it->getKick();
}

void StationaryMapState::calculateTurnAngle()
{
  if (d_selectedKick != nullptr || !hasEnoughBallObservations() || d_goalEstimates.size() == 0)
  {
    d_turnAngleRads = 0.0;
    d_turnBallPos = Vector2d::Zero();
    return;
  }

  // TODO handle the case where we only see one goal post and it's labelled as belonging to the opponent
  // TODO some decisions about turning should only be made once the entire area has been surveyed -- no way to express this currently

  // Enumerate kicks to find the angle which requires the least turning and/or repositioning
  double closestAngle = std::numeric_limits<double>::max();
  Vector2d closestBallPos = {};
  bool foundTurn = false;
  shared_ptr<Kick const> turnForKick;

  for (GoalEstimate const& goal : d_goalEstimates)
  {
    // Don't turn towards our goal
    if (goal.getLabel() == GoalLabel::Ours)
      continue;

    log::info("StationaryMapState::calculateTurnAngle") << "Evaluate goal at " << goal.getPost1Pos().transpose() << " to " << goal.getPost2Pos().transpose();

    // Find desirable target positions
    vector<Vector2d> targetPositions = {
      goal.getMidpoint(0.2),
      goal.getMidpoint(0.3),
      goal.getMidpoint(0.4),
      goal.getMidpoint(0.5),
      goal.getMidpoint(0.6),
      goal.getMidpoint(0.7),
      goal.getMidpoint(0.8)
    };

    for (shared_ptr<Kick const> const& kick : Kick::getAll())
    {
      Vector2d ballPos = kick->getIdealBallPos();
      Maybe<Vector2d> endPos = kick->estimateEndPos(ballPos);
      ASSERT(endPos.hasValue());
      double angle = Math::angleToPoint(*endPos);
      for (Vector2d const& targetPosition : targetPositions)
      {
        double targetAngle = Math::angleToPoint(targetPosition);

        // Check that there's no obstruction before the goal line -- blocks inside the goal are fine
        double occlusionDistance = d_occlusionMap.getOcclusionDistance(targetAngle);
        double goalLineDistance = getGoalLineDistance(goal, targetPosition);

        // TODO the entire ball must actually cross the line, not just its midpoint
        if (occlusionDistance < goalLineDistance)
        {
          log::info("StationaryMapState::calculateTurnAngle") << "Obstacle blocks kick " << kick->getId() << " at angle " << round(Math::radToDeg(targetAngle));
          continue;
        }

        if (fabs(closestAngle) > fabs(angle - targetAngle))
        {
          closestAngle = angle - targetAngle;
          closestBallPos = ballPos;
          turnForKick = kick;
          foundTurn = true;
          log::info("StationaryMapState::calculateTurnAngle") << "Turn " << Math::radToDeg(-closestAngle) << " degrees for '" << kick->getId() << "' to kick ball at " << closestBallPos.transpose() << " at " << Math::radToDeg(targetAngle) << " degrees to " << endPos->transpose() << " best yet";
        }
      }
    }
  }

  if (foundTurn)
  {
    d_turnAngleRads = -closestAngle;
    d_turnBallPos = closestBallPos;
    d_turnForKick = turnForKick;
    log::info("StationaryMapState::calculateTurnAngle") << "turn " << Math::radToDeg(d_turnAngleRads) << " degrees with ball at " << d_turnBallPos.transpose();
  }
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
        writer.String("label").Uint(static_cast<uint>(estimate.getLabel()));
      }
      writer.EndObject();
    }
    writer.EndArray();

    writer.String("goals").StartArray();
    for (auto const& estimate : d_goalEstimates)
    {
      writer.StartObject();
      {
        writer.String("post1").StartArray().Double(estimate.getPost1Pos().x()).Double(estimate.getPost1Pos().y()).EndArray();
        writer.String("post2").StartArray().Double(estimate.getPost2Pos().x()).Double(estimate.getPost2Pos().y()).EndArray();
        writer.String("label").Uint(static_cast<uint>(estimate.getLabel()));
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

    writer.String("openField");
    d_occlusionMap.writeJson(writer);

    writer.String("turnAngle").Double(d_turnAngleRads);
    writer.String("turnBallPos").StartArray().Double(d_turnBallPos.x()).Double(d_turnBallPos.y()).EndArray();
  }
  writer.EndObject();
}

bool StationaryMapState::needMoreSightingsOfGoalPostAt(Vector2d goalPos) const
{
  for (auto const& goalPostEstimate : d_goalPostEstimates)
  {
    double dist = (goalPostEstimate.getAverage() - goalPos).norm();
    if (dist < GoalPostMergeDistance)
      return goalPostEstimate.getCount() < GoalSamplesNeeded;
  }
  return false;
}

bool StationaryMapState::needMoreSightingsOfBallAt(Vector2d ballPos) const
{
  for (auto const& ballEstimate : d_ballEstimates)
  {
    double dist = (ballEstimate.getAverage() - ballPos).norm();
    if (dist < BallMergeDistance)
      return ballEstimate.getCount() < BallSamplesNeeded;
  }
  return false;
}

bool StationaryMapState::hasBallWithinDistance(double distance) const
{
  for (auto const& ballEstimate : d_ballEstimates)
  {
    if (ballEstimate.getCount() >= BallSamplesNeeded && ballEstimate.getAverage().norm() <= distance)
      return true;
  }
  return false;
}
