#ifndef BOLD_LINE_DETECT_PASS_HH
#define BOLD_LINE_DETECT_PASS_HH

#include <opencv2/core/core.hpp>
#include <Eigen/Core>

#include <memory>
#include <vector>

#include "../imagepasshandler.hh"
#include "../../HoughLineAccumulator/houghlineaccumulator.hh"
#include "../../HoughLineExtractor/houghlineextractor.hh"
#include "../../LineRunTracker/lineruntracker.hh"
#include "../../PixelLabel/pixellabel.hh"

namespace bold
{
  template <typename T>
  class LineDotPass : public ImagePassHandler<T>
  {
  private:
    const int d_imageWidth;
    std::shared_ptr<PixelLabel> const inLabel;
    std::shared_ptr<PixelLabel> const onLabel;
    const uchar hysterisisLimit; // TODO learn/control this variable (from config at least)
    LineRunTracker* d_rowTracker;
    std::vector<bold::LineRunTracker> d_colTrackers;

  public:
    std::vector<Eigen::Vector2i> lineDots;

    LineDotPass(int imageWidth, std::shared_ptr<PixelLabel> const inLabel, std::shared_ptr<PixelLabel> const onLabel, uchar hysterisisLimit)
    : d_imageWidth(imageWidth),
      lineDots(),
      inLabel(inLabel),
      onLabel(onLabel),
      hysterisisLimit(hysterisisLimit)
    {
      d_colTrackers = std::vector<bold::LineRunTracker>();

      for (int x = 0; x <= imageWidth; ++x)
      {
        d_colTrackers.push_back(bold::LineRunTracker(
          inLabel->id(), onLabel->id(), /*otherCoordinate*/x, hysterisisLimit,
          [this](ushort from, ushort to, ushort other) mutable {
            int mid = (from + to) / 2;
            lineDots.push_back(Eigen::Vector2i((int)other, mid));
          }
        ));
      }

      // TODO delete in destructor
      d_rowTracker = new LineRunTracker(
        inLabel->id(), onLabel->id(), /*otherCoordinate*/0, hysterisisLimit,
        [this](ushort from, ushort to, ushort other) mutable {
          int mid = (from + to) / 2;
          lineDots.push_back(Eigen::Vector2i(mid, (int)other));
        }
      );
    }

    void onImageStarting()
    {
      // reset all run trackers
      d_rowTracker->reset();
      for (int x = 0; x < d_imageWidth; ++x)
        d_colTrackers[x].reset();

      lineDots.clear();
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
  };
}

#endif
