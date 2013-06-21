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
    std::vector<std::shared_ptr<Control const>> d_controls;
    uchar d_hysterisisLimit;

  public:
    std::vector<Eigen::Vector2i> lineDots;

    LineDotPass(ushort imageWidth, std::shared_ptr<PixelLabel> const inLabel, std::shared_ptr<PixelLabel> const onLabel, uchar hysterisisLimit)
    : d_imageWidth(imageWidth),
      inLabel(inLabel),
      onLabel(onLabel),
      d_hysterisisLimit(hysterisisLimit),
      lineDots()
    {
      // Create trackers

      d_colTrackers = std::vector<bold::LineRunTracker>();

      for (ushort x = 0; x <= imageWidth; ++x)
      {
        d_colTrackers.push_back(bold::LineRunTracker(
          inLabel->id(), onLabel->id(), /*otherCoordinate*/x, d_hysterisisLimit,
          [this](ushort const from, ushort const to, ushort const other) mutable {
            int mid = (from + to) / 2;
            lineDots.push_back(Eigen::Vector2i((int)other, mid));
          }
        ));
      }

      // TODO delete in destructor
      d_rowTracker = new LineRunTracker(
        inLabel->id(), onLabel->id(), /*otherCoordinate*/0, d_hysterisisLimit,
        [this](ushort const from, ushort const to, ushort const other) mutable {
          int mid = (from + to) / 2;
          lineDots.push_back(Eigen::Vector2i(mid, (int)other));
        }
      );

      // Create controls

      auto hysterisisControl = Control::createInt(
        "Line Dot Hysterisis",
        [this]() { return d_hysterisisLimit; },
        [this](int const& value) mutable
        {
          d_hysterisisLimit = value;
          d_rowTracker->setHysterisisLimit(d_hysterisisLimit);
          for (LineRunTracker& colTracker : d_colTrackers)
            colTracker.setHysterisisLimit(d_hysterisisLimit);
        }
      );
      hysterisisControl->setLimitValues(0, 255);
      hysterisisControl->setIsAdvanced(true);
      d_controls.push_back(hysterisisControl);
    }

    std::vector<std::shared_ptr<Control const>> getControls() const { return d_controls; }

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
