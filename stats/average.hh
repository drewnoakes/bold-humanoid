#pragma once

#include "traits.hh"

namespace bold
{
  template<typename T>
  class Average
  {
  public:
    Average()
    {
      reset();
    }

    void reset()
    {
      d_count = 0;
      AveragingTraits::zero(d_sum);
      AveragingTraits::zero(d_average);
    }

    void add(T value)
    {
      d_sum += value;
      d_count++;
      d_average = d_sum / d_count;
    }

    T getAverage() const { return d_average; }

    int getCount() const { return d_count; }

  private:
    int d_count;
    T d_sum;
    T d_average;
  };
}
