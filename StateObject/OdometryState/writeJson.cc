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
    writer.Double(d_translation.x());
    writer.Double(d_translation.y());
    writer.Double(d_translation.z());
    writer.EndArray();
  }
  writer.EndObject();
}
