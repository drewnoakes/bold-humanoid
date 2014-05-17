#include "stationarymapstate.hh"

using namespace bold;
using namespace Eigen;
using namespace rapidjson;
using namespace std;

void StationaryMapState::writeJson(Writer<StringBuffer>& writer) const
{
  auto writeEstimates = [&writer](vector<Average<Vector3d>> const& estimates, string name)
  {
    writer.String(name.c_str()).StartArray();
    for (auto const& estimate : estimates)
    {
      writer.StartObject();
      {
        writer.String("pos").StartArray().Double(estimate.getAverage().x()).Double(estimate.getAverage().y()).EndArray();
        writer.String("count").Int(estimate.getCount());
      }
      writer.EndObject();
    }
    writer.EndArray();
  };

  writer.StartObject();
  {
    writeEstimates(d_ballEstimates, "balls");
    writeEstimates(d_goalEstimates, "goals");
    writeEstimates(d_teammateEstimates, "teammates");
  }
  writer.EndObject();
}
