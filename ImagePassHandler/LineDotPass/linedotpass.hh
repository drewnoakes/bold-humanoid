#pragma once

#include <opencv2/core/core.hpp>
#include <Eigen/Core>

#include <memory>
#include <vector>

#include "../imagepasshandler.hh"
#include "../../LineRunTracker/lineruntracker.hh"

namespace bold
{
  class PixelLabel;
  class ImageLabelData;
  class SequentialTimer;

  class LineDotPass : public ImagePassHandler
  {
  public:
    LineDotPass(ushort imageWidth, std::shared_ptr<PixelLabel> inLabel, std::shared_ptr<PixelLabel> onLabel);

    void process(ImageLabelData const& labelData, SequentialTimer& timer) override;

    std::vector<Eigen::Vector2i> lineDots;

  private:
    const ushort d_imageWidth;
    std::shared_ptr<PixelLabel> d_inLabel;
    std::shared_ptr<PixelLabel> d_onLabel;
    std::unique_ptr<LineRunTracker> d_rowTracker;
    std::vector<bold::LineRunTracker> d_colTrackers;
    int d_lastXGranularity;
  };
}
