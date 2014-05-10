#pragma once

#include <memory>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace bold
{
  class StateObject
  {
  protected:
    virtual ~StateObject() = default;

  public:
    virtual void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const = 0;

    static void writeJsonOrNull(rapidjson::Writer<rapidjson::StringBuffer>& writer, std::shared_ptr<StateObject const> const& stateObject)
    {
      if (stateObject)
        stateObject->writeJson(writer);
      else
        writer.Null();
    }
  };
}
