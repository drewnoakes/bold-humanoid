#pragma once

#include <cassert>

namespace bold
{
  template<typename T>
  class Range
  {
  public:
    Range() : d_isEmpty(true), d_min(), d_max() {}
    Range(T min, T max) : d_isEmpty(false), d_min(min), d_max(max) { assert(min <= max); }
    
    bool isEmpty() const { return d_isEmpty; }
    T min() const { return d_min; }
    T max() const { return d_max; }
    
    void expand(T value)
    {
      if (d_isEmpty)
      {
        d_isEmpty = false;
        d_min = value;
        d_max = value;
      }
      else
      {
        if (d_min > value)
          d_min = value;
        if (d_max < value)
          d_max = value;
      }
    }
    
    void reset()
    {
      d_isEmpty = true;
    }
    
    bool operator==(Range<T> const& other) const
    {
      if (d_isEmpty && other.d_isEmpty)
        return true;
      return
        d_isEmpty == other.d_isEmpty &&
        d_min == other.d_min &&
        d_max == other.d_max;
    }
    
    bool operator!=(Range<T> const& other) const
    {
      return !((*this) == other);
    }
    
  private:
    bool d_isEmpty;
    T d_min;
    T d_max;
  };
}