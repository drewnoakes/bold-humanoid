#pragma once

#include "../stats/movingaverage.hh"
#include "../Clock/clock.hh"
#include "../util/assert.hh"

namespace bold
{
  template<int WindowSize>
  class FPS
  {
  public:
    FPS()
    : d_avg(WindowSize),
      d_last(0)
    {}

    double next()
    {
      auto now = Clock::getTimestamp();

      if (!unlikely(d_last == 0 || now < d_last))
      {
        auto delta = now - d_last;
        d_avg.next(Clock::timestampToSeconds(delta));
      }

      d_last = now;

      return likely(d_avg.isMature()) ? 1.0/d_avg.getAverage() : 0.0;
    }

  private:
    MovingAverage<double> d_avg;
    Clock::Timestamp d_last;
  };
}
