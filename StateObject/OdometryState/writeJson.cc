#include "odometrystate.hh"

using namespace bold;
using namespace Eigen;
using namespace rapidjson;

void OdometryState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("tr");
    writer.StartArray();
    for (unsigned j = 0; j < 4; ++j)
    {
      for (unsigned i = 0; i < 4; ++i)
        writer.Double(d_transform.matrix()(i, j), "%.3f");
      writer.String(" ");
    }
    writer.EndArray();
  }
  writer.EndObject();
}
