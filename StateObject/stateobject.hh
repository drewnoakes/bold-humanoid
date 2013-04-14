#ifndef BOLD_STATE_HH
#define BOLD_STATE_HH

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <string>

namespace bold
{
  enum class StateType
  {
    AgentFrame,
    Alarm,
    Body,
    CameraFrame,
    Game,
    Hardware,
    WorldFrame
  };

  class StateObject
  {
  public:
//     virtual StateType type() const = 0;

    std::string name() const { return d_name; };

    virtual void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const {};

  protected:
    StateObject(std::string name)
    : d_name(name)
    {};

    ~StateObject()
    {};

  private:
    std::string d_name;
  };
}

#endif