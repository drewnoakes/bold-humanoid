#pragma once

#include "../stateobject.hh"
#include "../../Colour/colour.hh"
#include "../../util/json.hh"

#include <memory>
#include <vector>

namespace bold
{
  typedef std::pair<double, std::string> EventTiming;

  class TimingState : public StateObject
  {
  protected:
    TimingState(std::shared_ptr<std::vector<EventTiming>> eventTimings, ulong cycleNumber, double averageFps)
    : d_eventTimings(eventTimings),
      d_cycleNumber(cycleNumber),
      d_averageFps(averageFps)
    {}

    virtual ~TimingState() {}

  public:
    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }

    std::shared_ptr<std::vector<EventTiming> const> getTimings() const { return d_eventTimings; }

    ulong getCycleNumber() const { return d_cycleNumber; }
    double getAverageFps() const { return d_averageFps; }

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer>& writer) const;

    std::shared_ptr<std::vector<EventTiming>> d_eventTimings;
    ulong d_cycleNumber;
    double d_averageFps;
  };

  template<typename TBuffer>
  inline void TimingState::writeJsonInternal(rapidjson::Writer<TBuffer>& writer) const
  {
    writer.StartObject();
    {
      writer.String("cycle");
      writer.Uint64(d_cycleNumber);
      writer.String("fps");
      writer.Double(d_averageFps, "%.3f");
      writer.String("timings");
      writer.StartObject();
      {
        std::vector<EventTiming> const& timings = *d_eventTimings;
        for (EventTiming const& timing : timings)
        {
          writer.String(timing.second.c_str()); // event name
          writer.Double(timing.first, "%.3f");  // duration in milliseconds
        }
      }
      writer.EndObject();
    }
    writer.EndObject();
  }

  class MotionTimingState : public TimingState
  {
  public:
    MotionTimingState(std::shared_ptr<std::vector<EventTiming>> eventTimings, ulong cycleNumber, double averageFps)
    : TimingState(eventTimings, cycleNumber, averageFps)
    {}
  };

  class ThinkTimingState : public TimingState
  {
  public:
    ThinkTimingState(std::shared_ptr<std::vector<EventTiming>> eventTimings, ulong cycleNumber, double averageFps)
    : TimingState(eventTimings, cycleNumber, averageFps)
    {}
  };
}
