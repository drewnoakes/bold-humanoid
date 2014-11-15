#pragma once

#include <Eigen/Core>

#include "../ImageLabelData/imagelabeldata.hh"

namespace bold
{
  class SequentialTimer;

  /**
   * Abstract base class for classes that process labelled pixel data.
   */
  template <typename TPixel>
  class ImagePassHandler
  {
  public:
    virtual ~ImagePassHandler() = default;

    virtual void process(ImageLabelData const& labelData, SequentialTimer& timer) = 0;

    virtual std::string id() const = 0;
  };
}
