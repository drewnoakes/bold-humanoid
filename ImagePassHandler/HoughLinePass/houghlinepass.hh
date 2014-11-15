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
      : ImagePassHandler("HoughLinePass"),
        d_thresholdDivisor(thresholdDivisor),
        accumulator(width, height, accumulatorHeight),
        lines()
    {}

    void process(ImageLabelData<uchar> const& labelData, SequentialTimer& timer) override
    {
      accumulator.clear();
      timer.timeEvent("Clear");

      for (auto const& row : labelData)
      {
        ushort x = 0;
        for (auto const& label : row)
        {
          if (label != 0)
            accumulator.add(x, row.imageY);
          x += row.granularity.x();
        }
      }
      timer.timeEvent("Process Rows");

      auto extractor = HoughLineExtractor();
      lines = extractor.findLines(accumulator, accumulator.count() / d_thresholdDivisor);
      timer.timeEvent("Find lines");
    }

  private:
    int d_thresholdDivisor;
  };
}
