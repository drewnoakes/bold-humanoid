#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <opencv2/core/core.hpp>

namespace bold
{
  typedef unsigned int uint;
  
  class HoughLineAccumulator
  {
  private:
    /** Width of the accumulator matrix, spanning values of theta */
    uint const d_accumulatorThetaLen;
    /** height of the accumulator matrix, spanning values of radius */
    uint const d_accumulatorRadiusLen;

    /** Width of the input image */
    uint const d_xLength;
    /** Height of the input image */
    uint const d_yLength;

    // TODO multiplex these caches
    /** Cached sine values across the integer range [0-d_accumulatorThetaLen). */
    double* const d_sinCache;
    /** Cached cosine values across the integer range [0-d_accumulatorThetaLen). */
    double* const d_cosCache;

    cv::Mat d_accumulator;

    int d_count;

  public:
    HoughLineAccumulator(uint xLength, uint yLength, uint accumulatorHeight = 180);
    ~HoughLineAccumulator();

    /** The number of times 'add' was called since construction, or the last call to 'clear'. */
    int count() { return d_count; }

    void add(int x, int y);

    void clear();

    /** The accumulator matrix, containing vote counts for line hypotheses. */
    cv::Mat getMat() const;

    double getTheta(int y) const;
    double getRadius(int x) const;
  };
}
