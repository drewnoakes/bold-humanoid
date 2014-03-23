#pragma once

#include <opencv2/core/core.hpp>

#include <vector>
#include <memory>
#include <mutex>

#include "../PixelLabel/pixellabel.hh"

namespace bold
{
  class SequentialTimer;
  class Spatialiser;

  class ImageLabeller
  {
  public:
    ImageLabeller(std::shared_ptr<Spatialiser> spatialiser);

    ImageLabeller(std::shared_ptr<uchar const> const& lut, std::shared_ptr<Spatialiser> spatialiser);

    /** Replaces the LUT used by this image labeller. */
    void updateLut(std::shared_ptr<uchar const> const& lut);

    /**
     * Labels an entire image's pixels.
     *
     * @param image The input, colour image.
     * @param labelled The target image, in which labels are stored per-pixel.
     */
    void label(cv::Mat const& image, cv::Mat& labelled, SequentialTimer& timer, std::function<Eigen::Vector2i(int)> granularityFunction, bool ignoreAboveHorizon = false) const;

    /**
     * Generates an image in which each pixel is coloured according to the label
     * assigned to it. The result looks like a cartoon, or paint-by-numbers.
     */
    static void createCartoon(cv::Mat& labelledInput, cv::Mat& cartoonOutput, std::vector<bold::PixelLabel> const& labels);

  private:
    std::shared_ptr<uchar const> d_LUT;
    std::shared_ptr<Spatialiser> d_spatialiser;
    mutable std::mutex d_lutMutex;
  };
}
