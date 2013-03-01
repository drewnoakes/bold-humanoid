#ifndef BOLD_HOUGH_LINE_PASS_HH
#define BOLD_HOUGH_LINE_PASS_HH

#include <opencv2/core/core.hpp>
#include <vector>

#include "../imagepasshandler.hh"
#include "../../HoughLineAccumulator/houghlineaccumulator.hh"
#include "../../HoughLineExtractor/houghlineextractor.hh"

namespace bold
{
  /**
   * Processes an image, treating any non-zero pixels as though they're lines.
   * Intended for use on grayscale input images.
   */
  template <typename T>
  class HoughLinePass : public ImagePassHandler<T>
  {
  private:
    int d_thresholdDivisor;

  public:
    bold::HoughLineAccumulator accumulator;
    std::vector<bold::HoughLine> lines;

    HoughLinePass(int width, int height, int thresholdDivisor, int accumulatorHeight)
    : accumulator(width, height, accumulatorHeight),
      lines(),
      d_thresholdDivisor(thresholdDivisor)
    {}

    void onImageStarting()
    {
      accumulator.clear();
    }

    void onPixel(T value, int x, int y)
    {
      if (value != 0)
      {
        accumulator.add(x, y);
      }
    }

    void onImageComplete()
    {
      auto extractor = bold::HoughLineExtractor();

      lines = extractor.findLines(accumulator, accumulator.count() / d_thresholdDivisor);
    }
  };
}

#endif