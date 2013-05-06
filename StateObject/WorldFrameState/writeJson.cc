#include "worldframestate.hh"

using namespace bold;
using namespace rapidjson;

void WorldFrameState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("pos");
    writer.StartArray();
    {
      writer.Double(d_position.x());
      writer.Double(d_position.y());
      writer.Double(d_position.theta());
    }
    writer.EndArray();

    writer.String("ball");
    if (d_ballObservation.hasValue())
    {
      writer.StartArray();
      writer.Double(d_ballObservation->x());
      writer.Double(d_ballObservation->y());
      writer.Double(d_ballObservation->z());
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
        writer.Double(goalPos.x());
        writer.Double(goalPos.y());
        writer.Double(goalPos.z());
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
        writer.Double(lineSeg.p1().x());
        writer.Double(lineSeg.p1().y());
        writer.Double(lineSeg.p1().z());
        writer.Double(lineSeg.p2().x());
        writer.Double(lineSeg.p2().y());
        writer.Double(lineSeg.p2().z());
        writer.EndArray();
      }
    }
    writer.EndArray();
  }
  writer.EndObject();
}
