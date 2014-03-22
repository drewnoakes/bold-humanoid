#pragma once

#include <opencv2/core/core.hpp>
#include <Eigen/Core>

#include <memory>
#include <vector>

#include "../imagepasshandler.hh"
#include "../../Config/config.hh"
#include "../../HoughLineAccumulator/houghlineaccumulator.hh"
#include "../../HoughLineExtractor/houghlineextractor.hh"
#include "../../LineRunTracker/lineruntracker.hh"
#include "../../PixelLabel/pixellabel.hh"
#include "../../SequentialTimer/sequentialtimer.hh"

namespace bold
{
  template <typename T>
  class LineDotPass : public ImagePassHandler<T>
  {
  private:
    const ushort d_imageWidth;
    std::shared_ptr<PixelLabel> const inLabel;
    std::shared_ptr<PixelLabel> const onLabel;
    std::unique_ptr<LineRunTracker> d_rowTracker;
    std::vector<bold::LineRunTracker> d_colTrackers;

  public:
    std::vector<Eigen::Vector2i> lineDots;

    LineDotPass(ushort imageWidth, std::shared_ptr<PixelLabel> const inLabel, std::shared_ptr<PixelLabel> const onLabel)
    : d_imageWidth(imageWidth),
      inLabel(inLabel),
      onLabel(onLabel),
      lineDots()
    {
      auto hysteresisLimit = Config::getSetting<int>("vision.line-detection.line-dots.hysteresis");

      // Create trackers

      d_colTrackers = std::vector<bold::LineRunTracker>();

      for (ushort x = 0; x <= imageWidth; ++x)
      {
        d_colTrackers.emplace_back(
          inLabel->id(), onLabel->id(), /*otherCoordinate*/x, hysteresisLimit->getValue(),
          [this](ushort const from, ushort const to, ushort const other) mutable {
            int mid = (from + to) / 2;
            lineDots.emplace_back((int)other, mid);
          }
        );
      }

      d_rowTracker = std::make_unique<LineRunTracker>(
        inLabel->id(), onLabel->id(), /*otherCoordinate*/0, hysteresisLimit->getValue(),
        [this](ushort const from, ushort const to, ushort const other) mutable {
          int mid = (from + to) / 2;
          lineDots.emplace_back(mid, (int)other);
        }
      );

      // Create controls

      hysteresisLimit->changed.connect([this](int const& value) mutable
      {
        d_rowTracker->setHysteresisLimit(value);

        for (LineRunTracker& colTracker : d_colTrackers)
          colTracker.setHysteresisLimit(value);
      });
    }

    void onImageStarting(SequentialTimer& timer) override
    {
      // reset all run trackers
      d_rowTracker->reset();
      for (ushort x = 0; x < d_imageWidth; ++x)
        d_colTrackers[x].reset();

      lineDots.clear();

      timer.timeEvent("Clear");
    }

    void onRowStarting(ushort y, Eigen::Vector2i const& granularity) override
    {
      d_rowTracker->reset();
      d_rowTracker->otherCoordinate = y;
    }

    void onPixel(T label, ushort x, ushort y) override
    {
      d_rowTracker->update(label, x);
      d_colTrackers[x].update(label, y);
    }

    std::string id() const override
    {
      return std::string("LineDotPass");
    }
  };
}
