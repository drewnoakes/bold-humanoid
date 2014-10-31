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

void SettingBase::writeFullJson(Writer<WebSocketBuffer>& writer) const
{
  writeJsonInternal(writer);
}

void SettingBase::writeFullJson(Writer<StringBuffer>& writer) const
{
  writeJsonInternal(writer);
}

void SettingBase::triggerChanged() const
{
  changedBase(this);
}
