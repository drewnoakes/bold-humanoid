#pragma once

#include "../stateobject.hh"
#include "../../Colour/colour.hh"

#include <iostream>
#include <memory>
#include <vector>

namespace bold
{
  typedef std::pair<double, std::string> EventTiming;
  
  class TimingState : public StateObject
  {
  protected:
    TimingState(std::shared_ptr<std::vector<EventTiming>> eventTimings)
    : d_eventTimings(eventTimings)
    {}
    
    virtual ~TimingState() {}

  public:
    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override
    {
      writer.StartObject();
      {
        std::vector<EventTiming> const& timings = *d_eventTimings;
        for (EventTiming const& timing : timings)
        {
          writer.String(timing.second.c_str()); // event name
          writer.Double(timing.first);          // duration in milliseconds
        }
      }
      writer.EndObject();      
    }
    
    std::shared_ptr<std::vector<EventTiming> const> getTimings() const { return d_eventTimings; }
    
  private:
   std::shared_ptr<std::vector<EventTiming>> d_eventTimings;
  };
  
  class MotionTimingState : public TimingState
  {
  public:
    MotionTimingState(std::shared_ptr<std::vector<EventTiming>> eventTimings)
    : TimingState(eventTimings)
    {}
  };
  
  class ThinkTimingState : public TimingState
  {
  public:
    ThinkTimingState(std::shared_ptr<std::vector<EventTiming>> eventTimings)
    : TimingState(eventTimings)
    {}
  };
}
