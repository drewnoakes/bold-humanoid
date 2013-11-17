#pragma once

#include <cassert>
#include <iostream>

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

    void expand(Range<T> value)
    {
      if (value.isEmpty())
        return;

      expand(value.min());
      expand(value.max());
    }

    void reset()
    {
      d_isEmpty = true;
    }

    T size() const
    {
      return d_max - d_min;
    }

    bool contains(T value) const
    {
      return d_min <= value && d_max >= value;
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

  template<typename T>
  std::ostream& operator<<(std::ostream &stream, Range<T> const& range)
  {
    if (range.isEmpty())
      return stream << "(empty)";
    return stream << "(" << range.min() << " -> " << range.max() << ")";
  }
}
