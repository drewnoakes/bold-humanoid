#pragma once

#include <opencv2/core/core.hpp>
#include <Eigen/Core>

#include <memory>
#include <vector>

#include "../imagepasshandler.hh"
#include "../../Control/control.hh"
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
    const ushort d_imageWidth;
    std::shared_ptr<PixelLabel> const inLabel;
    std::shared_ptr<PixelLabel> const onLabel;
    LineRunTracker* d_rowTracker;
    std::vector<bold::LineRunTracker> d_colTrackers;
    Control d_hysterisisControl;

  public:
    std::vector<Eigen::Vector2i> lineDots;

    LineDotPass(ushort imageWidth, std::shared_ptr<PixelLabel> const inLabel, std::shared_ptr<PixelLabel> const onLabel, uchar hysterisisLimit)
    : d_imageWidth(imageWidth),
      inLabel(inLabel),
      onLabel(onLabel),
      lineDots()
    {
      d_colTrackers = std::vector<bold::LineRunTracker>();

      for (ushort x = 0; x <= imageWidth; ++x)
      {
        d_colTrackers.push_back(bold::LineRunTracker(
          inLabel->id(), onLabel->id(), /*otherCoordinate*/x, hysterisisLimit,
          [this](ushort const from, ushort const to, ushort const other) mutable {
            int mid = (from + to) / 2;
            lineDots.push_back(Eigen::Vector2i((int)other, mid));
          }
        ));
      }

      // TODO delete in destructor
      d_rowTracker = new LineRunTracker(
        inLabel->id(), onLabel->id(), /*otherCoordinate*/0, hysterisisLimit,
        [this](ushort const from, ushort const to, ushort const other) mutable {
          int mid = (from + to) / 2;
          lineDots.push_back(Eigen::Vector2i(mid, (int)other));
        }
      );

      d_hysterisisControl = Control::createInt(
        "Line Dot Hysterisis",
        hysterisisLimit,
        [this](int const& value) mutable
        {
          d_rowTracker->setHysterisisLimit(value);
          for (LineRunTracker& colTracker : d_colTrackers)
            colTracker.setHysterisisLimit(value);
        }
      );
      d_hysterisisControl.setLimitValues(0, 255);
      d_hysterisisControl.setIsAdvanced(true);
    }

    Control getHysterisisControl() const { return d_hysterisisControl; }

    void onImageStarting() override
    {
      // reset all run trackers
      d_rowTracker->reset();
      for (ushort x = 0; x < d_imageWidth; ++x)
        d_colTrackers[x].reset();

      lineDots.clear();
    }

    void onRowStarting(ushort y) override
    {
      d_rowTracker->reset();
      d_rowTracker->otherCoordinate = y;
    }

    void onPixel(T label, ushort x, ushort y) override
    {
      d_rowTracker->update(label, x);
      d_colTrackers[x].update(label, y);
    }
  };
}
