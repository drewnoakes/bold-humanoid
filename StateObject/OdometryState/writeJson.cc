#include "odometrystate.hh"

using namespace bold;
using namespace Eigen;
using namespace rapidjson;

void OdometryState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("translation");
    writer.StartArray();
    for (unsigned i = 0; i < 4; ++i)
      for (unsigned j = 0; j < 4; ++j)
        writer.Double(d_transform.matrix()(i, j));
    writer.EndArray();
  }
  writer.EndObject();
}
