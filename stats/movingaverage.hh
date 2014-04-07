#pragma once

#include <cmath>
#include <vector>
#include <stdexcept>
#include <string.h>
#include <Eigen/Core>

namespace bold
{
  /** Compile time test whether a type has a \a fill member
   *
   * Works thanks to SFINAE (Substitution Failure Is Not An Error): if
   * U::fill does not exist, \a decltype does not give a valid type to
   * substitute as an argument, which removes the first template
   * method from valid options. Otherwise, it passes and is chosen as
   * being more specific than the second overload.
   */
  template<typename T>
  struct has_fill_member
  {
    typedef char yes;
    typedef long no;

    template<typename U>
    static yes test(decltype(&U::fill));
                                       
    template<typename U>
    static no test(...);
                       
    static const bool value = sizeof(test<T>(0)) == sizeof(yes);  
  };

  /// Traits used to select correct method of zero-ing the used type
  struct AveragingTraits
  {
    template<typename T>
    static void zero(T& v, typename std::enable_if<has_fill_member<T>::value>::type* dummy = 0) { v.fill(0); }
    
    template<typename T>
    static void zero(T& v, typename std::enable_if<std::is_arithmetic<T>::value>::type* dummy = 0) { v = 0; }
  };

  template<typename T>
  class MovingAverage
  {
  public:
    MovingAverage(unsigned short windowSize)
    : d_items(new T[windowSize]),
      d_length(0),
      d_nextPointer(0),
      d_sum(),
      d_windowSize(windowSize)
    {
      if (windowSize == 0)
        throw new std::runtime_error("Cannot have zero window size.");
      AveragingTraits::zero(d_sum);
    }

    MovingAverage(MovingAverage const& other)
    {
      copy(other);
    }

    ~MovingAverage()
    {
      destroy();
    }

    MovingAverage& operator=(MovingAverage const& other)
    {
      destroy();
      copy(other);
      return *this;
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
      
      d_avg = d_sum / int{d_length};
      
      return d_avg;
    }

    void reset()
    {
      d_length = 0;
      d_nextPointer = 0;
      d_sum = {};
    }

    T getAverage() const { return d_avg; }

    T calculateStdDev()
    {
      // TODO unit test this
      T sum;
      AveragingTraits::zero(sum);
      for (int i = 0; i < d_length; i++)
      {
        int index = (d_nextPointer - i - 1) % d_windowSize;
        if (index < 0)
          index += d_windowSize;
        assert(index >= 0 && index < d_length);
        T diff = d_items[index] - d_avg;
        sum += diff * diff;
      }
      return sqrt(sum / d_length);
    }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  private:
    T* d_items;
    unsigned short d_length;
    int d_nextPointer;
    T d_sum;
    T d_avg;
    unsigned short d_windowSize;

    void destroy()
    {
      delete[] d_items;
    }
    
    void copy(MovingAverage const& other)
    {
      d_windowSize = other.d_windowSize;
      d_items = new T[other.d_windowSize];
      for (unsigned i = 0; i < d_windowSize; ++i)
        d_items[i] = other.d_items[i];

      d_length = other.d_length;
      d_nextPointer = other.d_nextPointer;
      d_sum = other.d_sum;
      d_avg = other.d_avg;
    }
  };
}
