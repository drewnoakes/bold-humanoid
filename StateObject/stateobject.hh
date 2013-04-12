#ifndef BOLD_STATE_HH
#define BOLD_STATE_HH

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace bold
{
  enum class StateType
  {
    CameraFrame,
    AgentFrame,
    Game,
    Hardware,
    Body,
    Alarm
  };

  class StateObject
  {
  public:
//     virtual StateType type() const = 0;

    // TODO could be a pure virtual function eventually
    virtual void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
    {}

  protected:
    StateObject()
    {};

    ~StateObject()
    {};
  };
}

#endif