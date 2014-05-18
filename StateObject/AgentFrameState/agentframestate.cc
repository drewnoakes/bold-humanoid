#include "agentframestate.hh"

using namespace bold;
using namespace Eigen;
using namespace rapidjson;

AgentFrameState::AgentFrameState(
  Maybe<Eigen::Vector3d> ballObservation,
  std::vector<Eigen::Vector3d> goalObservations,
  std::vector<Eigen::Vector3d> teamMateObservations,
  std::vector<LineSegment3d> observedLineSegments,
  std::vector<LineJunction, Eigen::aligned_allocator<LineJunction>> observedLineJunctions,
  Maybe<Polygon2d> visibleFieldPoly,
  std::vector<std::pair<Eigen::Vector3d,Eigen::Vector3d>> occlusionRays,
  ulong thinkCycleNumber)
: d_ballObservation(std::move(ballObservation)),
  d_goalObservations(std::move(goalObservations)),
  d_teamMateObservations(std::move(teamMateObservations)),
  d_observedLineSegments(std::move(observedLineSegments)),
  d_observedLineJunctions(std::move(observedLineJunctions)),
  d_visibleFieldPoly(std::move(visibleFieldPoly)),
  d_occlusionRays(std::move(occlusionRays)),
  d_thinkCycleNumber(thinkCycleNumber)
{}

bool AgentFrameState::shouldSeeAgentFrameGroundPoint(Vector2d groundAgent) const
{
  // TODO accept a 3d vector, and use similar triangles to transform such that z == 0
  return d_visibleFieldPoly->contains(groundAgent);
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
        writer.StartArray();
        writer.Double(ray.first.x(), "%.3f");
        writer.Double(ray.first.y(), "%.3f");
        writer.Double(ray.second.x(), "%.3f");
        writer.Double(ray.second.y(), "%.3f");
        writer.EndArray();
      }
    }
    writer.EndArray();
  }
  writer.EndObject();
}
