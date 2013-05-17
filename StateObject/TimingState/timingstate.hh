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
      writer.String("timings");
      writer.StartObject();
      {
        std::vector<EventTiming> const& timings = *d_eventTimings;
        for (EventTiming const& timing : timings)
        {
          writer.String(timing.second.c_str());
          writer.Double(timing.first * 1000.0);
        }
      }
      writer.EndObject();      
    }
    
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
