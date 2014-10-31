#include "agentframestate.hh"

using namespace bold;
using namespace Eigen;
using namespace rapidjson;
using namespace std;

AgentFrameState::AgentFrameState(
  Maybe<Vector3d> ballObservation,
  vector<Vector3d> goalObservations,
  vector<Vector3d> teamMateObservations,
  vector<LineSegment3d> observedLineSegments,
  vector<LineJunction, aligned_allocator<LineJunction>> observedLineJunctions,
  Maybe<Polygon2d> visibleFieldPoly,
  vector<OcclusionRay<double>> occlusionRays,
  ulong thinkCycleNumber)
: d_ballObservation(move(ballObservation)),
  d_goalObservations(move(goalObservations)),
  d_teamMateObservations(move(teamMateObservations)),
  d_observedLineSegments(move(observedLineSegments)),
  d_observedLineJunctions(move(observedLineJunctions)),
  d_visibleFieldPoly(move(visibleFieldPoly)),
  d_occlusionRays(move(occlusionRays)),
  d_thinkCycleNumber(thinkCycleNumber)
{}

Maybe<Vector3d> AgentFrameState::getClosestGoalObservation() const
{
  if (d_goalObservations.empty())
    return Maybe<Vector3d>::empty();

  auto closestGoalDist = numeric_limits<double>::max();
  Maybe<Vector3d> closest;

  for (auto const& obs : d_goalObservations)
  {
    auto dist = obs.head<2>().norm();
    if (dist < closestGoalDist)
    {
      closestGoalDist = dist;
      closest = obs;
    }
  }

  return closest;
}

bool AgentFrameState::shouldSeeAgentFrameGroundPoint(Vector2d groundAgent) const
{
  // TODO accept a 3d vector, and use similar triangles to transform such that z == 0
  return d_visibleFieldPoly->contains(groundAgent);
}

double AgentFrameState::getOcclusionDistance(double angle) const
{
  // Compute the position at which where 'angle' could be inserted without changing the ordering.
  // This means that *(lower - 1) is below, and *lower is above.
  auto lower = lower_bound(
    d_occlusionRays.begin(),
    d_occlusionRays.end(),
    angle,
    [](OcclusionRay<double> const& ray, double const& a)
    {
      return Math::angleToPoint(ray.near()) < a;
    });

  if (lower == d_occlusionRays.begin() || lower == d_occlusionRays.end())
    return numeric_limits<double>::quiet_NaN();

  LineSegment2d occlusionEdge((lower - 1)->near(), lower->near());
  LineSegment2d ray(Vector2d::Zero(), Vector2d(-sin(angle), cos(angle)));

  double u;
  double t;
  auto i = ray.tryIntersect(occlusionEdge, t, u);

  return t <= 0 ? numeric_limits<double>::quiet_NaN() : t;
}

bool AgentFrameState::isNearBall(Vector2d point, double maxDistance) const
{
  return d_ballObservation.hasValue() && ((point - d_ballObservation->head<2>()).norm() < maxDistance);
}

bool AgentFrameState::isNearGoal(Vector2d point, double maxDistance) const
{
  for (auto const& goal : d_goalObservations)
  {
    if ((point - goal.head<2>()).norm() < maxDistance)
      return true;
  }
  return false;
}

Maybe<Polygon2d> AgentFrameState::getOcclusionPoly() const
{
  return getOcclusionPoly(d_occlusionRays);
}

Maybe<Polygon2d> AgentFrameState::getOcclusionPoly(std::vector<OcclusionRay<double>> const& occlusionRays)
{
  auto nearPointsVec = Polygon2d::PointVector{};
  auto farPointsVec = Polygon2d::PointVector{};
  for (auto const& ray : occlusionRays)
  {
    nearPointsVec.push_back(ray.near());
    farPointsVec.push_back(ray.far());
  }
  std::reverse(begin(nearPointsVec), end(nearPointsVec));

  auto pointsVec = Polygon2d::PointVector{};
  for (auto const& p : nearPointsVec)
    pointsVec.push_back(p);
  for (auto const& p : farPointsVec)
    pointsVec.push_back(p);

  return make_polygon2d(pointsVec);
}
