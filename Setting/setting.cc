#include "setting-implementations.hh"

#include "../Config/config.hh"

using namespace bold;
using namespace rapidjson;
using namespace std;

bool SettingBase::isInitialising()
{
  return Config::isInitialising();
}

void SettingBase::writeFullJson(Writer<StringBuffer>& writer) const
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
