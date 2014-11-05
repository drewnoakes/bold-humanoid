#pragma once

#include <opencv2/core/core.hpp>
#include <vector>

#include "../imagepasshandler.hh"
#include "../../HoughLineAccumulator/houghlineaccumulator.hh"
#include "../../HoughLineExtractor/houghlineextractor.hh"
#include "../../SequentialTimer/sequentialtimer.hh"

namespace bold
{
  /**
   * Processes an image, treating any non-zero pixels as though they're lines.
   * Intended for use on grayscale input images.
   */
  template <typename T>
  class HoughLinePass : public ImagePassHandler<T>
  {
  public:
    HoughLineAccumulator accumulator;
    std::vector<Candidate<Line>> lines;

    HoughLinePass(uint width, uint height, int thresholdDivisor, uint accumulatorHeight)
    : d_thresholdDivisor(thresholdDivisor),
      accumulator(width, height, accumulatorHeight),
      lines()
    {}

    void onImageStarting(SequentialTimer& timer) override
    {
      accumulator.clear();

      timer.timeEvent("Clear");
    }

    void onPixel(T value, ushort x, ushort y) override
    {
      if (value != 0)
      {
        accumulator.add(x, y);
      }
    }

    void onImageComplete(SequentialTimer& timer) override
    {
      auto extractor = HoughLineExtractor();

      lines = extractor.findLines(accumulator, accumulator.count() / d_thresholdDivisor);

      timer.timeEvent("Find lines");
    }

    std::string id() const override
    {
      return std::string("HoughLinePass");
    }

  private:
    int d_thresholdDivisor;
  };
}
