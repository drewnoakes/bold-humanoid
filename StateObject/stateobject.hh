#pragma once

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace bold
{
  class StateObject
  {
  public:
    virtual void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const = 0;
  };
}
