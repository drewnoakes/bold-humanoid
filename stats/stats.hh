#pragma once

#include <iterator>
#include <cmath>

namespace bold
{
  template<typename T, typename Iterator>
  T average(Iterator from, Iterator end)
  {
    int count = 0;
    T sum = {0};

    for (auto it = from; it < end; it++)
    {
      sum += *it;
      count++;
    }

    return sum / count;
  }

  template<typename T, typename Iterator>
  T variance(Iterator from, Iterator end)
  {
    static_assert(std::is_floating_point<T>::value, "Only floating point types supported at present");

    T average = ::bold::average<T>(from, end);
    T sumDevs = {0};
    int count = 0;

    for (auto it = from; it < end; it++)
    {
      sumDevs += pow(*it - average, 2);
      count++;
    }

    T variance = sumDevs/count;

    return variance;
  }

  template<typename T, typename Iterator>
  T stdDev(Iterator from, Iterator end)
  {
    static_assert(std::is_floating_point<T>::value, "Only floating point types supported at present");

    return sqrt(::bold::variance<T>(from, end));
  }
}
