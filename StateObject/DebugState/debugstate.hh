#pragma once

#include "../stateobject.hh"

#include <iostream>
#include <memory>
#include <vector>

namespace bold
{
  typedef std::pair<double,std::string> EventTiming;

  class DebugState : public StateObject
  {
  public:
    DebugState(std::shared_ptr<std::vector<EventTiming>> eventTimings, unsigned gameControllerMessageCount, unsigned ignoredMessageCount)
    : d_eventTimings(eventTimings),
      d_gameControllerMessageCount(gameControllerMessageCount),
      d_ignoredMessageCount(ignoredMessageCount)
    {}

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    std::shared_ptr<std::vector<EventTiming>> d_eventTimings;
    unsigned d_gameControllerMessageCount;
    unsigned d_ignoredMessageCount;
  };
}
