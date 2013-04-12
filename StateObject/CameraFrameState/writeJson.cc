#include "cameraframestate.hh"

using namespace bold;
using namespace rapidjson;

void CameraFrameState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("ball");
    if (d_ballObservation.hasValue())
    {
      writer.StartArray();
      writer.Double(d_ballObservation.value()->x());
      writer.Double(d_ballObservation.value()->y());
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
        writer.EndArray();
      }
    }
    writer.EndArray();

    writer.String("observedLineSegments");
    writer.StartArray();
    {
      for (LineSegment2i const& lineSeg : d_observedLineSegments)
      {
        writer.StartArray();
        writer.Double(lineSeg.p1().x());
        writer.Double(lineSeg.p1().y());
        writer.Double(lineSeg.p2().x());
        writer.Double(lineSeg.p2().y());
        writer.EndArray();
      }
    }
    writer.EndArray();
  }
  writer.EndObject();
}