#include "cameraframestate.hh"

using namespace bold;
using namespace rapidjson;

void CameraFrameState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("thinkCycle").Uint64(d_thinkCycleNumber);

    writer.String("ball");
    if (d_ballObservation.hasValue())
    {
      writer.StartArray();
      writer.Double(d_ballObservation->x(), "%.1f");
      writer.Double(d_ballObservation->y(), "%.1f");
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
        writer.Double(goalPos.x(), "%.1f");
        writer.Double(goalPos.y(), "%.1f");
        writer.EndArray();
      }
    }
    writer.EndArray();

    writer.String("lines");
    writer.StartArray();
    {
      for (LineSegment2i const& lineSeg : d_observedLineSegments)
      {
        writer.StartArray();
        writer.Double(lineSeg.p1().x(), "%.1f");
        writer.Double(lineSeg.p1().y(), "%.1f");
        writer.Double(lineSeg.p2().x(), "%.1f");
        writer.Double(lineSeg.p2().y(), "%.1f");
        writer.EndArray();
      }
    }
    writer.EndArray();

    writer.String("totalPixelCount").Uint64(d_totalPixelCount);
    writer.String("processedPixelCount").Uint64(d_processedPixelCount);
  }
  writer.EndObject();
}
