#ifndef BOLD_STATE_HH
#define BOLD_STATE_HH

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace bold
{
  class StateObject
  {
  protected:
    StateObject()
    {};

    ~StateObject()
    {};

    // TODO could be a pure virtual function eventually
    virtual void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
    {}
  };
}

#endif