#pragma once

#include <cmath>
#include <vector>
#include <stdexcept>
#include <string.h>

namespace bold
{
  template<typename T>
  class MovingAverage
  {
  public:
    MovingAverage(int windowSize)
    : d_items(windowSize),
      d_length(0),
      d_nextPointer(0),
      d_windowSize(windowSize)
    {
      if (windowSize <= 0)
        throw new std::runtime_error("Window size must be greater than zero.");
      memset(&d_sum, 0, sizeof(d_sum));
    }

    int count() const { return d_length; }
    int getWindowSize() const { return d_windowSize; }
    bool isMature() const { return d_length == d_windowSize; }

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

      d_avg = d_sum / d_length;

      return d_avg;
    }

    T getAverage() const { return d_avg; }

    T calculateStdDev()
    {
      // TODO unit test this
      T sum = 0;
      for (int i = 0; i < d_length; i++)
      {
        int index = (d_nextPointer - i - 1) % d_windowSize;
        T diff = d_items[index] - d_avg;
        sum += diff * diff;
      }
      return sqrt(sum / d_windowSize);
    }

  private:
    std::vector<T> d_items;
    int d_length;
    int d_nextPointer;
    T d_sum;
    T d_avg;
    int d_windowSize;
  };
}
