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

void AgentFrameState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("thinkCycle").Uint64(d_thinkCycleNumber);

    writer.String("ball");
    if (d_ballObservation.hasValue())
    {
      writer.StartArray();
      writer.Double(d_ballObservation->x(), "%.3f");
      writer.Double(d_ballObservation->y(), "%.3f");
      writer.Double(d_ballObservation->z(), "%.3f");
      writer.EndArray();
    }
    else
    {
      writer.Null();
    }

    writer.String("goals");
    writer.StartArray();
    {
      for (auto const& goalPos : d_goalObservations)
      {
        writer.StartArray();
        writer.Double(goalPos.x(), "%.3f");
        writer.Double(goalPos.y(), "%.3f");
        writer.Double(goalPos.z(), "%.3f");
        writer.EndArray();
      }
    }
    writer.EndArray();

    writer.String("teammates");
    writer.StartArray();
    {
      for (auto const& teamMatePos : d_teamMateObservations)
      {
        writer.StartArray();
        writer.Double(teamMatePos.x(), "%.3f");
        writer.Double(teamMatePos.y(), "%.3f");
        writer.Double(teamMatePos.z(), "%.3f");
        writer.EndArray();
      }
    }
    writer.EndArray();

    writer.String("lines");
    writer.StartArray();
    {
      for (auto const& lineSeg : d_observedLineSegments)
      {
        writer.StartArray();
        writer.Double(lineSeg.p1().x(), "%.3f");
        writer.Double(lineSeg.p1().y(), "%.3f");
        writer.Double(lineSeg.p1().z(), "%.3f");
        writer.Double(lineSeg.p2().x(), "%.3f");
        writer.Double(lineSeg.p2().y(), "%.3f");
        writer.Double(lineSeg.p2().z(), "%.3f");
        writer.EndArray();
      }
    }
    writer.EndArray();

    writer.String("junctions");
    writer.StartArray();
    {
      for (auto const& junction : d_observedLineJunctions)
      {
        writer.StartObject();
        writer.String("p")
          .StartArray()
          .Double(junction.position(0), "%.3f")
          .Double(junction.position(1), "%.3f")
          .EndArray();
        writer.String("a")
          .Double(junction.angle);
        writer.String("t")
          .Uint(static_cast<unsigned>(junction.type));
        writer.EndObject();
      }
    }
    writer.EndArray();

    writer.String("visibleFieldPoly");
    writer.StartArray();
    {
      if (d_visibleFieldPoly.hasValue())
      {
        for (auto const& vertex : d_visibleFieldPoly.value())
        {
          writer.StartArray();
          writer.Double(vertex.x(), "%.3f");
          writer.Double(vertex.y(), "%.3f");
          writer.EndArray();
        }
      }
    }
    writer.EndArray();

    writer.String("occlusionRays");
    writer.StartArray();
    {
      for (auto const& ray : d_occlusionRays)
      {
        // Should be enough to check a single value for NaN
        if (std::isnan(ray.far().x()))
          continue;

        writer.StartArray();
        writer.Double(ray.near().x(), "%.3f");
        writer.Double(ray.near().y(), "%.3f");
        writer.Double(ray.far().x(), "%.3f");
        writer.Double(ray.far().y(), "%.3f");
        writer.EndArray();
      }
    }
    writer.EndArray();
  }
  writer.EndObject();
}
