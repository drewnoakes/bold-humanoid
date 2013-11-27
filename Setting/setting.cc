#include "setting-implementations.hh"

#include "../Config/config.hh"

using namespace bold;

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
    if (isAdvanced())
      writer.String("advanced").Bool(true);
    if (isReadOnly())
      writer.String("readonly").Bool(true);
    writer.String("value");
    writeJsonValue(writer);
    writeJsonMetadata(writer);
  }
  writer.EndObject();
}
