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
    TimingState(std::shared_ptr<std::vector<EventTiming>> eventTimings, ulong cycleNumber)
    : d_eventTimings(eventTimings),
      d_cycleNumber(cycleNumber)
    {}

    virtual ~TimingState() {}

  public:
    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override
    {
      writer.StartObject();
      {
        writer.String("cycle").Uint64(d_cycleNumber);
        writer.String("timings");
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
      writer.EndObject();
    }

    std::shared_ptr<std::vector<EventTiming> const> getTimings() const { return d_eventTimings; }

    ulong getCycleNumber() const { return d_cycleNumber; }

  private:
   std::shared_ptr<std::vector<EventTiming>> d_eventTimings;
   ulong d_cycleNumber;
  };

  class MotionTimingState : public TimingState
  {
  public:
    MotionTimingState(std::shared_ptr<std::vector<EventTiming>> eventTimings, ulong cycleNumber)
    : TimingState(eventTimings, cycleNumber)
    {}
  };

  class ThinkTimingState : public TimingState
  {
  public:
    ThinkTimingState(std::shared_ptr<std::vector<EventTiming>> eventTimings, ulong cycleNumber)
    : TimingState(eventTimings, cycleNumber)
    {}
  };
}
