#include "setting-implementations.hh"

#include "../Config/config.hh"

using namespace bold;
using namespace std;

bool SettingBase::isInitialising()
{
  return Config::isInitialising();
}

void SettingBase::writeFullJson(rapidjson::Writer< rapidjson::StringBuffer >& writer) const
{
  writer.StartObject();
  {
    writer.String("path").String(getPath().c_str());
    writer.String("type").String(getTypeName().c_str());
    if (getDescription().size())
      writer.String("description").String(getDescription().c_str());
    if (isReadOnly())
      writer.String("readonly").Bool(true);
    writer.String("value");
    writeJsonValue(writer);
    writeJsonMetadata(writer);
  }
  writer.EndObject();
}

ostream& bold::operator<<(ostream &stream, vector<string> const& strings)
{
  stream <<  "[";
  bool comma = false;
  for (auto const& s : strings)
  {
    if (comma)
      stream << ",";
    comma = true;
    stream << "\"" << s << "\"";
  }
  stream <<  "]";
  return stream;
}
