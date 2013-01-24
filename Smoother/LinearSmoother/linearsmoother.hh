#ifndef BOLD_LINEARSMOOTHER_HH
#define BOLD_LINEARSMOOTHER_HH

#include "../smoother.hh"

namespace bold
{
  class LinearSmoother : public virtual Smoother
  {
  private:
    double d_delta;
  public:
    LinearSmoother(double initialValue, double delta)
    : Smoother(initialValue),
      d_delta(delta)
    {}

    void step()
    {
      double diff = d_target - d_current;

      if (diff == 0)
        return;

      if (diff > d_delta)
        d_current += d_delta;
      else if (diff < d_delta)
        d_current -= d_delta;
      else
        d_current = d_target;
    }
  };
}

#endif