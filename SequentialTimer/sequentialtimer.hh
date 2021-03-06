#pragma once

#include <vector>
#include <memory>
#include <deque>
#include <sstream>

#include "../Clock/clock.hh"
#include "../util/assert.hh"

namespace bold
{
  typedef std::pair<double, std::string> EventTiming;

  class SequentialTimer
  {
  public:
    SequentialTimer()
    : d_eventTimings(std::make_shared<std::vector<EventTiming>>()),
      d_prefix(""),
      d_last(Clock::getTimestamp()),
      d_flushed(false)
    {}

    void timeEvent(std::string const& eventName)
    {
      ASSERT(!d_flushed);
      auto now = Clock::getTimestamp();
      auto timeMillis = Clock::timestampToMillis(now - d_last);
      std::string name = d_prefix.size() ? d_prefix + '/' + eventName : eventName;
      d_eventTimings->emplace_back(timeMillis, name);
      d_last = now;
    }

    std::shared_ptr<std::vector<EventTiming>> flush()
    {
      ASSERT(d_entered.size() == 0);
      d_flushed = true;
      return d_eventTimings;
    }

    void enter(std::string name)
    {
      auto now = Clock::getTimestamp();
      d_last = now;
      d_entered.emplace_back(name, now);
      rebuildPrefix();
    }

    void exit()
    {
      ASSERT(d_entered.size() != 0);
      auto now = Clock::getTimestamp();
      auto top = d_entered[d_entered.size() - 1];
      auto timeMillis = Clock::timestampToMillis(now - top.second);
      d_eventTimings->emplace_back(timeMillis, d_prefix);
      d_last = now;
      d_entered.pop_back();
      rebuildPrefix();
    }

    std::string getPrefix() const { return d_prefix; }

  private:
    void rebuildPrefix()
    {
      std::stringstream prefix;
      bool first = true;
      for (auto const& e : d_entered)
      {
        if (first)
          first = false;
        else
          prefix << '/';
        prefix << e.first;
      }
      d_prefix = prefix.str();
    }

    std::shared_ptr<std::vector<EventTiming>> d_eventTimings;
    std::deque<std::pair<std::string,Clock::Timestamp>> d_entered;
    std::string d_prefix;
    Clock::Timestamp d_last;
    bool d_flushed;
  };
}
