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
    writer.Double(d_translation.x(), "%.4f");
    writer.Double(d_translation.y(), "%.4f");
    writer.Double(d_translation.z(), "%.4f");
    writer.EndArray();
  }
  writer.EndObject();
}
