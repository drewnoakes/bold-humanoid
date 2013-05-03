#pragma once

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
    HoughLineAccumulator accumulator;
    std::vector<Candidate<Line>> lines;

    HoughLinePass(int width, int height, int thresholdDivisor, int accumulatorHeight)
    : d_thresholdDivisor(thresholdDivisor),
      accumulator(width, height, accumulatorHeight),
      lines()
    {}

    void onImageStarting() override
    {
      accumulator.clear();
    }

    void onPixel(T value, int x, int y) override
    {
      if (value != 0)
      {
        accumulator.add(x, y);
      }
    }

    void onImageComplete() override
    {
      auto extractor = HoughLineExtractor();

      lines = extractor.findLines(accumulator, accumulator.count() / d_thresholdDivisor);
    }
  };
}
