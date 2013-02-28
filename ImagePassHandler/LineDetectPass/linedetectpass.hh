#ifndef BOLD_LINE_DETECT_PASS_HH
#define BOLD_LINE_DETECT_PASS_HH

#include <opencv2/core/core.hpp>
#include <vector>

#include "../imagepasshandler.hh"
#include "../../HoughLineAccumulator/houghlineaccumulator.hh"
#include "../../HoughLineExtractor/houghlineextractor.hh"
#include "../../LineRunTracker/lineruntracker.hh"

namespace bold
{
  template <typename T>
  class LineDetectPass : public ImagePassHandler<T>
  {
  private:
    int d_thresholdDivisor;
    const int d_imageWidth;
    const int d_imageHeight;
    const uchar inLabel;
    const uchar onLabel;
    const uchar hysterisisLimit; // TODO learn/control this variable (from config at least)
    LineRunTracker* d_rowTracker;
    std::vector<bold::LineRunTracker> d_colTrackers;

  public:
    bold::HoughLineAccumulator accumulator;
    std::vector<bold::HoughLine> lines;

    LineDetectPass(int width, int height, int thresholdDivisor, int accumulatorHeight, const uchar inLabel, const uchar onLabel, const uchar hysterisisLimit)
    : accumulator(width, height, accumulatorHeight),
      d_imageWidth(width),
      d_imageHeight(height),
      lines(),
      d_thresholdDivisor(thresholdDivisor),
      inLabel(inLabel),
      onLabel(onLabel),
      hysterisisLimit(hysterisisLimit)
    {
      d_colTrackers = std::vector<bold::LineRunTracker>();

      for (int x = 0; x <= width; ++x)
      {
        d_colTrackers.push_back(bold::LineRunTracker(
          inLabel, onLabel, /*otherCoordinate*/x, hysterisisLimit,
          [&](ushort const from, ushort const to, ushort const other) {
            int mid = (from + to) / 2;
            //d_lineDots.add(Eigen::Vector2i((int)other, mid));
            accumulator.add((int)other, (int)mid);
          }
        ));
      }

      // TODO delete in desctructor
      d_rowTracker = new LineRunTracker(
        inLabel, onLabel, /*otherCoordinate*/0, hysterisisLimit,
        [&](ushort const from, ushort const to, ushort const other) {
          int mid = (from + to) / 2;
          //d_lineDots.add(Eigen::Vector2i(mid, (int)other));
          accumulator.add((int)mid, (int)other);
        }
      );
    }

    void onImageStarting()
    {
      // reset all run trackers
      d_rowTracker->reset();
      for (int x = 0; x < d_imageWidth; ++x)
        d_colTrackers[x].reset();

      // clear the accumulator
      accumulator.clear();
    }

    void onRowStarting(int y)
    {
      d_rowTracker->reset();
      d_rowTracker->otherCoordinate = y;
    }

    void onPixel(T label, int x, int y)
    {
      d_rowTracker->update(label, x);
      d_colTrackers[x].update(label, y);
    }

    void onImageComplete()
    {
      auto extractor = bold::HoughLineExtractor();

      lines = extractor.findLines(accumulator, accumulator.count() / d_thresholdDivisor);
    }
  };
}

#endif