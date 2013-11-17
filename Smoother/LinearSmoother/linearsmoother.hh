#pragma once

#include "../smoother.hh"

#include <cmath>

namespace bold
{
  class LinearSmoother : public virtual Smoother
  {
  public:
    LinearSmoother(double initialValue, double delta)
    : Smoother(initialValue),
      d_delta(delta)
    {}

    void step() override
    {
      double diff = d_target - d_current;

      if (diff == 0)
        return;

      // Limit the rate of change to delta

      if (diff > 0)
        d_current += std::min(d_delta, diff);
      else
        d_current -= std::min(d_delta, -diff);
    }

    double getDelta() const { return d_delta; }
    void setDelta(double delta) { d_delta = delta; }

  private:
    double d_delta;
  };
}
