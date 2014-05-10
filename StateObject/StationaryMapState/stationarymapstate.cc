#include "stationarymapstate.hh"

using namespace bold;
using namespace Eigen;

void StationaryMapState::writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("balls").StartArray();
    for (auto const& ball : d_ballEstimates)
    {
      writer.StartObject();
      {
        writer.String("pos").StartArray().Double(ball.getAverage().x()).Double(ball.getAverage().y()).EndArray();
        writer.String("count").Int(ball.getCount());
      }
      writer.EndObject();
    }
    writer.EndArray();

    writer.String("goals").StartArray();
    for (auto const& goal : d_goalEstimates)
    {
      writer.StartObject();
      {
        writer.String("pos").StartArray().Double(goal.getAverage().x()).Double(goal.getAverage().y()).EndArray();
        writer.String("count").Int(goal.getCount());
      }
      writer.EndObject();
    }
    writer.EndArray();
  }
  writer.EndObject();
}
