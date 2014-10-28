#pragma once

#include <rapidjson/document.h>

namespace bold
{
  inline int GetIntWithDefault(const rapidjson::Value& val, const char* name, int defaultValue)
  {
    auto member = val.FindMember(name);
    if (member == val.MemberEnd() || !member->value.IsInt())
      return defaultValue;
    return member->value.GetInt();
  }

  inline unsigned int GetUintWithDefault(const rapidjson::Value& val, const char* name, unsigned int defaultValue)
  {
    auto member = val.FindMember(name);
    if (member == val.MemberEnd() || !member->value.IsUint())
      return defaultValue;
    return member->value.GetUint();
  }

  inline double GetDoubleWithDefault(const rapidjson::Value& val, const char* name, double defaultValue)
  {
    auto member = val.FindMember(name);
    if (member == val.MemberEnd() || !member->value.IsDouble())
      return defaultValue;
    return member->value.GetDouble();
  }

  inline bool GetBoolWithDefault(const rapidjson::Value& val, const char* name, bool defaultValue)
  {
    auto member = val.FindMember(name);
    if (member == val.MemberEnd() || !member->value.IsBool())
      return defaultValue;
    return member->value.GetBool();
  }

  inline const char* GetStringWithDefault(const rapidjson::Value& val, const char* name, const char* defaultValue)
  {
    auto member = val.FindMember(name);
    if (member == val.MemberEnd() || !member->value.IsString())
      return defaultValue;
    return member->value.GetString();
  }
}
