#ifndef BOLD_HOUGHLINEACCUMULATOR_HH
#define BOLD_HOUGHLINEACCUMULATOR_HH

#include <iostream>
#include <vector>
#include <cmath>
#include <opencv2/core/core.hpp>

namespace bold
{
  class HoughLineAccumulator
  {
  private:
    /** Width of the accumulator matrix, spanning values of theta */
    unsigned int const d_accumulatorThetaLen;
    /** height of the accumulator matrix, spanning values of radius */
    unsigned int const d_accumulatorRadiusLen;

    /** Width of the input image */
    unsigned int const d_xLength;
    /** Height of the input image */
    unsigned int const d_yLength;

    /** Cached sine values across the integer range [0-d_accumulatorThetaLen). */
    double* const d_sinCache;
    /** Cached cosine values across the integer range [0-d_accumulatorThetaLen). */
    double* const d_cosCache;

    /** The accumulator matrix, containing vote counts for line hypotheses. */
    cv::Mat d_accumulator;

  public:
    HoughLineAccumulator(unsigned int xLength, unsigned int yLength, unsigned int accumulatorWidth = 180);
    ~HoughLineAccumulator();

    void add(int x, int y);

    void clear();

    cv::Mat getMat();

    double getTheta(unsigned int y);
    double getRadius(unsigned int x);
  };
}

#endif
