#pragma once

#include <vector>
#include <stdexcept>

namespace bold
{
  template<typename T>
  class MovingAverage
  {
  public:
    MovingAverage(unsigned windowSize)
    : d_items(windowSize),
      d_length(0),
      d_nextPointer(0),
      d_windowSize(windowSize)
    {
      if (windowSize == 0)
        throw new std::runtime_error("Cannot have zero window size.");
      memset(&d_sum, 0, sizeof(d_sum));
    }

    T next(T value)
    {
      if (d_length == d_windowSize)
      {
        d_sum -= d_items[d_nextPointer];
      }
      else
      {
        d_length++;
      }

      d_items[d_nextPointer] = value;
      d_sum += value;
      d_nextPointer = (d_nextPointer + 1) % d_windowSize;

      return d_sum / d_length;
    }

  private:
    std::vector<T> d_items;
    unsigned d_length;
    int d_nextPointer;
    T d_sum;
    unsigned d_windowSize;
  };
}
