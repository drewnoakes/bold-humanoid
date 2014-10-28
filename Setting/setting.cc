#include "setting-implementations.hh"

#include "../Config/config.hh"

using namespace bold;
using namespace rapidjson;
using namespace std;

SettingBase::SettingBase(string path, string typeName, type_index typeIndex, bool isReadOnly, string description)
: d_path(path),
  d_typeName(typeName),
  d_typeIndex(typeIndex),
  d_isReadOnly(isReadOnly),
  d_description(description)
{
  auto last = d_path.find_last_of('.');
  auto nameStart = last == string::npos
    ? 0
    : last + 1;
  d_name = d_path.substr(nameStart);
}

bool SettingBase::isInitialising()
{
  return Config::isInitialising();
}

void SettingBase::writeFullJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("path");
    writer.String(getPath().c_str());
    writer.String("type");
    writer.String(getTypeName().c_str());
    if (getDescription().size())
    {
      writer.String("description");
      writer.String(getDescription().c_str());
    }
    if (isReadOnly())
    {
      writer.String("readonly");
      writer.Bool(true);
    }
    writer.String("value");
    writeJsonValue(writer);
    writeJsonMetadata(writer);
  }
  writer.EndObject();
}

void SettingBase::triggerChanged() const
{
  changedBase(this);
}
