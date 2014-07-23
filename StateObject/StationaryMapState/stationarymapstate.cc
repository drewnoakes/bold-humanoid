#include "stationarymapstate.hh"

#include "../../Config/config.hh"
#include "../../FieldMap/fieldmap.hh"
#include "../../geometry/LineSegment/LineSegment2/linesegment2.hh"
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

bool GoalEstimate::isTowards(double ballEndAngle) const
{
  LineSegment2d kickPath(Vector2d::Zero(), Math::pointAtAngle(ballEndAngle + (M_PI/2.0), 1000.0));
  LineSegment2d goalLine(d_post1Pos, d_post2Pos);

  double goalEdgeAvoidRatio = 0.1;

  double t;
  double u;
  auto intersection = kickPath.tryIntersect(goalLine, t, u);

  double ratio = u / FieldMap::getGoalY();


  return intersection.hasValue() && ratio > goalEdgeAvoidRatio && ratio < (1 - goalEdgeAvoidRatio);
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

  return GoalEstimate(d_post1Pos + fieldX, d_post2Pos + fieldX, label);
}

///////////////////////////////////////////////////////////////////////////////

StationaryMapState::StationaryMapState(
  std::vector<Average<Vector2d>> ballEstimates,
  std::vector<Average<Vector2d>> goalPostEstimates,
  std::vector<Average<Vector2d>> keeperEstimates,
  RadialOcclusionMap occlusionMap)
: d_ballEstimates(ballEstimates),
  d_goalPostEstimates(goalPostEstimates),
  d_keeperEstimates(keeperEstimates),
  d_occlusionMap(occlusionMap)
{
  // Sort estimates such that those with greater numbers of observations appear first
  std::sort(d_ballEstimates.begin(), d_ballEstimates.end(), compareAverages<Vector2d>);
  std::sort(d_keeperEstimates.begin(), d_keeperEstimates.end(), compareAverages<Vector2d>);
  std::sort(d_goalPostEstimates.begin(), d_goalPostEstimates.end(), compareAverages<Vector2d>);

  // Convert the goal post estimates into estimated goals (pairs of posts)
  auto goalPairs = pairGoalPosts(d_goalPostEstimates);

  // Label the goal post pairs as either our goal, their goal or unknown
  for (auto const& pair : goalPairs)
  {
    Maybe<Vector2d> agentBallPos;
    if (d_ballEstimates.size() != 0 && d_ballEstimates[0].getCount() > BallSamplesNeeded)
      agentBallPos = d_ballEstimates[0].getAverage();
    auto label = labelGoal(pair.first, pair.second, agentBallPos);
    d_goalEstimates.emplace_back(pair.first.getAverage(), pair.second.getAverage(), label);
  }

  // If we only see our goal, synthesize an estimate of the opposite goal position
  if (d_goalEstimates.size() == 1)
  {
    auto const& goal = d_goalEstimates[0];
    if (goal.getLabel() == GoalLabel::Ours)
      d_goalEstimates.push_back(goal.estimateOppositeGoal(GoalLabel::Theirs));
  }
  else if (d_goalEstimates.size() > 2)
  {
    log::warning("StationaryMapState::StationaryMapState")
      << d_goalEstimates.size() << " goals detected from " << d_goalPostEstimates.size() << " goal posts";
  }

  // In general we should only see one goal.
  // Log warnings in some unexpected cases.
  auto theirCount = std::count_if(d_goalEstimates.begin(), d_goalEstimates.end(), [](GoalEstimate const& goal) { return goal.getLabel() == GoalLabel::Theirs; });
  auto ourCount   = std::count_if(d_goalEstimates.begin(), d_goalEstimates.end(), [](GoalEstimate const& goal) { return goal.getLabel() == GoalLabel::Ours; });

  if (theirCount > 1)
    log::warning("StationaryMapState::StationaryMapState")
      << "Detected " << theirCount << " occurrences of their goal (from " << d_goalPostEstimates.size() << " goal posts)";

  if (ourCount > 1)
    log::warning("StationaryMapState::StationaryMapState")
      << "Detected " << ourCount << " occurrences of our goal (from " << d_goalPostEstimates.size() << " goal posts)";

  // Only attempt to select a kick/turn angle if the ball is within a reasonable distance
  if (hasBallWithinDistance(0.5))
  {
    // Try to find a kick we can do immediately
    selectImmediateKick();

    // Try to select a turn and kick to perform
    calculateTurnAndKick();

    // Given enough observations, we should always attempt to do something... if not, log warning
    static bool errorFlag = false;
    if (hasEnoughBallAndGoalPostObservations())
    {
      if (!d_selectedKick && d_turnAngleRads == 0 && !errorFlag)
      {
        errorFlag = true;
        log::warning("StationaryMapState::StationaryMapState") << "Have enough observations, but no kick or turn was selected";
      }
    }
    else
    {
      errorFlag = false;
    }
  }
}

vector<pair<Average<Vector2d>,Average<Vector2d>>> StationaryMapState::pairGoalPosts(vector<Average<Vector2d>> goalPostEstimates)
{
  std::deque<Average<Vector2d>> posts(goalPostEstimates.begin(), goalPostEstimates.end());

  vector<pair<Average<Vector2d>,Average<Vector2d>>> pairs;

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

      // TODO error should be a function of distance, as uncertainty increases with distance

      static auto maxGoalPairDistanceError = Config::getSetting<double>("vision.goal-detection.max-pair-error-dist");

      if (error < maxGoalPairDistanceError->getValue())
      {
        pairs.emplace_back(post1, *post2);

        posts.erase(post2);
        break;
      }
    }
  }

  return pairs;
}

GoalLabel StationaryMapState::labelGoalByKeeperBallDistance(
  Average<Eigen::Vector2d> const& post1Pos,
  Average<Eigen::Vector2d> const& post2Pos,
  FieldSide ballSideEstimate)
{
  // If the keeper tells us that the ball is 1m from them, and we observe the
  // ball as 1.5m from the midpoint of the goal, then it's likely ours.

  const double maxPositionMeasurementError = 0.4; // TODO review this experimentally and move to config

  const double theirsThreshold = Vector2d(
    (FieldMap::getFieldLengthX() / 2.0) + maxPositionMeasurementError,
    FieldMap::getFieldLengthY() / 2.0
  ).norm();

  Vector2d const mid = (post1Pos.getAverage() + post2Pos.getAverage()) / 2.0;

  switch (ballSideEstimate)
  {
    case FieldSide::Ours:
    {
      // Keeper says the ball is on our side of the field.
      // If this goal is close enough then it must be ours, assuming the ball is approximately at our feet.
      double goalDist = mid.norm();
      if (goalDist < (FieldMap::getFieldLengthX() / 2.0) - maxPositionMeasurementError)
      {
        log::verbose("BuildStationaryMap::labelGoalObservations") << "Keeper believes ball is on our side, and closest goal is too close at " << goalDist;
        return GoalLabel::Ours;
      }
      break;
    }
    case FieldSide::Theirs:
    {
      // Keeper says the ball is on their side of the field.
      // If this goal is far enough then it must be ours, assuming the ball is approximately at our feet.
      double goalDist = mid.norm();
      if (goalDist > theirsThreshold)
      {
        log::verbose("BuildStationaryMap::labelGoalObservations") << "Keeper believes ball is on the opponent's side, and closest goal is too far at " << goalDist;
        return GoalLabel::Ours;
      }
      break;
    }
    default:
    {
      break;
    }
  }

  return GoalLabel::Unknown;
}

GoalLabel StationaryMapState::labelGoalByKeeperBallPosition(
  Average<Eigen::Vector2d> const& post1Pos,
  Average<Eigen::Vector2d> const& post2Pos,
  Eigen::Vector2d const& keeperBallPos,
  Eigen::Vector2d const& agentBallPos)
{
  static auto maxKeeperBallDistance = Config::getSetting<double>("vision.goal-detection.label.max-keeper-ball-dist");

  // Convert keeper's agent frame observation to world frame
  Vector2d keeperBallPosWorldFrame(
    (-FieldMap::getFieldLengthX() / 2.0) + keeperBallPos.y(),
    -keeperBallPos.x());

  Vector2d ballPosIfTheirs = estimateWorldPositionForPoint(post1Pos.getAverage(), post2Pos.getAverage(), agentBallPos, GoalLabel::Theirs);
  Vector2d ballPosIfOurs = estimateWorldPositionForPoint(post1Pos.getAverage(), post2Pos.getAverage(), agentBallPos, GoalLabel::Ours);

  double errorIfOurs   = (keeperBallPosWorldFrame - ballPosIfOurs).norm();
  double errorIfTheirs = (keeperBallPosWorldFrame - ballPosIfTheirs).norm();

  double dist = maxKeeperBallDistance->getValue();

  // If the two errors are too similar, then we cannot decide
  if (fabs(errorIfOurs - errorIfTheirs) < dist)
    return GoalLabel::Unknown;

  // Whichever has the lesser error
  return errorIfOurs < errorIfTheirs
    ? GoalLabel::Ours
    : GoalLabel::Theirs;
}

GoalLabel StationaryMapState::labelGoalByKeeperObservations(
  Average<Eigen::Vector2d> const& post1Pos,
  Average<Eigen::Vector2d> const& post2Pos,
  std::vector<Average<Eigen::Vector2d>> keeperEstimates)
{
  static auto maxGoalieGoalDistance = Config::getSetting<double>("vision.player-detection.max-goalie-goal-dist");

  Vector2d const mid = (post1Pos.getAverage() + post2Pos.getAverage()) / 2.0;

  // Label this goal based upon whether we see our keeper between the posts
  for (auto const& keeperEstimate : keeperEstimates)
  {
    // Stop looping when the estimates are not confident enough
    if (keeperEstimate.getCount() < KeeperSamplesNeeded)
      break;

    Vector2d keeperPos = keeperEstimate.getAverage();
    Vector2d distFromMid = mid - keeperPos;

    if (distFromMid.norm() < maxGoalieGoalDistance->getValue())
    {
      log::info("BuildStationaryMap::labelGoalObservations") << "Observe keeper between goal posts, so assume as ours";
      return GoalLabel::Ours;
    }
  }

  return GoalLabel::Unknown;
}

GoalLabel StationaryMapState::labelGoal(
  Average<Eigen::Vector2d> const& post1Pos,
  Average<Eigen::Vector2d> const& post2Pos,
  Maybe<Eigen::Vector2d> const& agentBallPos)
{
  GoalLabel label = GoalLabel::Unknown;

  auto team = State::get<TeamState>();
  if (team)
  {
    auto keeper = team->getKeeperState();

    if (keeper && keeper->ballRelative.hasValue())
      label = labelGoalByKeeperBallPosition(post1Pos, post2Pos, *keeper->ballRelative, agentBallPos.hasValue() ? *agentBallPos : Vector2d::Zero());

    if (label == GoalLabel::Unknown)
      label = labelGoalByKeeperBallDistance(post1Pos, post2Pos, team->getKeeperBallSideEstimate());
  }

  if (label == GoalLabel::Unknown)
    label = labelGoalByKeeperObservations(post1Pos, post2Pos, d_keeperEstimates);

  return label;
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

void StationaryMapState::selectImmediateKick()
{
  auto ball = std::find_if(
    d_ballEstimates.begin(),
    d_ballEstimates.end(),
    [](Average<Vector2d> b) { return b.getCount() >= BallSamplesNeeded && b.getAverage().norm() < 0.5; });

  if (ball == d_ballEstimates.end())
    return;

  auto goal = std::find_if(
    d_goalEstimates.begin(),
    d_goalEstimates.end(),
    [](GoalEstimate g) { return g.getLabel() != GoalLabel::Ours; });

  if (goal == d_goalEstimates.end())
    return;

  Vector2d ballPosAgent = ball->getAverage().head<2>();

  // Check each kick to see if it's possible, and whether it would give a good result
  for (auto& kick : Kick::getAll())
  {
    Maybe<Vector2d> ballEndPosAgent = kick->estimateEndPos(ballPosAgent);

    // If no end position, then this kick is not possible given the ball's current position
    if (!ballEndPosAgent.hasValue())
      continue;

    double ballEndAngle = Math::angleToPoint(*ballEndPosAgent);

    // Determine whether the end pos is advantageous
    bool isOnTarget = false;

    // Currently, only kick toward the goal
    // If the kick angle is not directed between the goal posts, skip it
    // TODO the end pos doesn't necessarily have to be between the goals -- sometimes just nearer the goal is enough
    if (goal->isTowards(ballEndAngle))
    {
      // Check that there's no obstruction before the goal line -- blocks inside the goal are fine
      double occlusionDistance = d_occlusionMap.getOcclusionDistance(ballEndAngle);
      double goalLineDistance = getGoalLineDistance(*goal, *ballEndPosAgent);

      // TODO the entire ball must actually cross the line, not just its midpoint
      if (occlusionDistance >= goalLineDistance)
      {
        isOnTarget = true;
        log::info("StationaryMapState::selectKick")
          << "Goal kick possible: " << kick->getId()
          << " angleDegs=" << Math::radToDeg(ballEndAngle)
          << " goalLabel=" << getGoalLabelName(goal->getLabel())
          << " occlusionDist=" << occlusionDistance
          << " goalLineDist=" << goalLineDistance;
      }
    }

    d_possibleKicks.emplace_back(kick, ballEndPosAgent.value(), isOnTarget);
  }

  // TODO when more than one kick is possible, take the best, not the first
  auto it = find_if(d_possibleKicks.begin(), d_possibleKicks.end(), [](KickResult const& k) { return k.isOnTarget(); });
  if (it != d_possibleKicks.end())
    d_selectedKick =  it->getKick();
}

void StationaryMapState::calculateTurnAndKick()
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

    log::info("StationaryMapState::calculateTurnAndKick") << "Evaluate goal at " << goal.getPost1Pos().transpose() << " to " << goal.getPost2Pos().transpose();

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
          log::info("StationaryMapState::calculateTurnAndKick") << "Obstacle blocks kick " << kick->getId() << " at angle " << round(Math::radToDeg(targetAngle));
          continue;
        }

        if (fabs(closestAngle) > fabs(angle - targetAngle))
        {
          closestAngle = angle - targetAngle;
          closestBallPos = ballPos;
          turnForKick = kick;
          foundTurn = true;
          log::info("StationaryMapState::calculateTurnAndKick") << "Turn " << Math::radToDeg(-closestAngle) << " degrees for '" << kick->getId() << "' to kick ball at " << closestBallPos.transpose() << " at " << Math::radToDeg(targetAngle) << " degrees to " << endPos->transpose() << " best yet";
        }
      }
    }
  }

  if (foundTurn)
  {
    d_turnAngleRads = -closestAngle;
    d_turnBallPos = closestBallPos;
    d_turnForKick = turnForKick;
    log::info("StationaryMapState::calculateTurnAndKick") << "turn " << Math::radToDeg(d_turnAngleRads) << " degrees with ball at " << d_turnBallPos.transpose();
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

Vector2d StationaryMapState::estimateWorldPositionForPoint(
  Vector2d const& post1,
  Vector2d const& post2,
  Vector2d const& pointAgent,
  GoalLabel label)
{
  ASSERT(label == GoalLabel::Ours || label == GoalLabel::Theirs);

  LineSegment2d goalLine(post1, post2);
  Vector2d mid = goalLine.mid() - pointAgent;

  Vector2d perp = Math::findPerpendicularVector(goalLine.delta());

  double x = mid.dot(perp.normalized());

  if (x < 0)
  {
    x = -x;
    perp = -perp;
  }

  double r = mid.norm();
  double y = sqrt(r*r - x*x);

  double aMid = Math::angleToPoint(Vector2d(-mid));
  double aPerp = Math::angleToPoint(Vector2d(-perp));

  if (Math::normaliseRads(aMid - aPerp) < 0)
    y = -y;

  if (label == GoalLabel::Theirs)
  {
    x = FieldMap::getFieldLengthX() / 2.0 - x;
    y = -y;
  }
  else
  {
    x -= FieldMap::getFieldLengthX() / 2.0;
  }

  return Vector2d(x, y);
}

long StationaryMapState::getSatisfactoryGoalPostCount() const
{
  return std::count_if(
    d_goalPostEstimates.begin(),
    d_goalPostEstimates.end(),
    [](Average<Eigen::Vector2d> const& estimate) { return estimate.getCount() > GoalSamplesNeeded; });
}
