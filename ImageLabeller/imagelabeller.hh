#pragma once

#include <opencv2/core/core.hpp>

#include <vector>
#include <memory>
#include <mutex>

#include "../PixelLabel/pixellabel.hh"

namespace bold
{
  class ImageSampleMap;
  class ImageLabelData;
  class SequentialTimer;
  class Spatialiser;

  enum class TeamColour
  {
    Cyan = 1,
    Magenta = 2
  };

  class ImageLabeller
  {
  public:
    ImageLabeller(std::shared_ptr<Spatialiser> spatialiser);

    ImageLabeller(std::shared_ptr<uchar const> const& lut, std::shared_ptr<Spatialiser> spatialiser);

    /** Replaces the LUT used by this image labeller. */
    void updateLut(std::shared_ptr<uchar const> const& lut);

    /** Labels image pixels according to the specified sample map. */
    ImageLabelData label(cv::Mat const& image, ImageSampleMap const& sampleMap, bool ignoreAboveHorizon, SequentialTimer& timer) const;

  private:
    std::shared_ptr<uchar const> d_LUT;
    std::shared_ptr<Spatialiser> d_spatialiser;
    mutable std::mutex d_lutMutex;
  };
}
