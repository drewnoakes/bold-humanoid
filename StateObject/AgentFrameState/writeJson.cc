#include "agentframestate.hh"

using namespace bold;
using namespace rapidjson;

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

    writer.String("lines");
    writer.StartArray();
    {
      for (LineSegment3d const& lineSeg : d_observedLineSegments)
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
