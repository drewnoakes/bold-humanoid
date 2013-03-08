#ifndef BOLD_DISTRIBUTION_TRACKER
#define BOLD_DISTRIBUTION_TRACKER

#include <stdexcept>
#include <cmath>

namespace bold
{
  class DistributionTracker
  {
  public:
    DistributionTracker()
    : d_sum(0),
      d_sumOfSquares(0),
      d_count(0)
    {}

    /** Returns all state of this {@link DistributionTracker}. */
    void reset() { d_sum = d_sumOfSquares = d_count = 0; };

    /** Integrates an observation into this tracker's internal state. */
    void add(double d)
    {
      if (std::isnan(d))
        throw std::runtime_error("Cannot add NaN to a DistributionTracker");
      d_sum += d;
      d_sumOfSquares += d*d;
      d_count++;
    }

    /** Returns the average of observed values. */
    double average() const
    {
      return d_count == 0 ? NAN : d_sum/d_count;
    }

    /** Returns the population variance for observed values. */
    double variance() const
    {
      double avg = average();
      return d_count == 0 ? NAN : d_sumOfSquares/d_count - avg*avg;
    }

    /** Returns the population standard deviation for observed values. */
    double stdDev() const
    {
      return sqrt(variance());
    }

    /** Returns the number of observed values. */
    unsigned count() const { return d_count; }

    /** Returns the sum of observed values. */
    double sum() const { return d_sum; }

    /** Returns the sum of squared observed values. */
    double sumOfSquares() const { return d_sumOfSquares; }

  private:
    double d_sum;
    double d_sumOfSquares;
    unsigned d_count;
  };
}

#endif