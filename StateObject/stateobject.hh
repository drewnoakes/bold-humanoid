#pragma once

#include <memory>

#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "../util/websocketbuffer.hh"

namespace bold
{
  /** Base class for all state objects in the system.
   *
   * State objects model some kind of state, including but not limited to state about the hardware,
   * behaviour, environment, game, team mates, etc.
   *
   * Instances are registered with the State class, and are immutable thereafter. This allows safe
   * use of state objects across threads.
   */
  class StateObject
  {
  protected:
    virtual ~StateObject() = default;

  public:
    virtual void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const = 0;
    virtual void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const = 0;
    virtual void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const = 0;

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
