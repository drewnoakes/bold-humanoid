#pragma once

#include <memory>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "../util/websocketbuffer.hh"

namespace bold
{
  class StateObject
  {
  protected:
    virtual ~StateObject() = default;

  public:
    virtual void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const = 0;
    virtual void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const = 0;

    template<typename TBuffer, typename TState>
    static void writeJsonOrNull(rapidjson::Writer<TBuffer>& writer, std::shared_ptr<TState const> const& stateObject)
    {
      static_assert(std::is_base_of<StateObject,TState>::value, "TState must be a descendant of StateObject");

      if (stateObject)
        stateObject->writeJson(writer);
      else
        writer.Null();
    }
  };
}
