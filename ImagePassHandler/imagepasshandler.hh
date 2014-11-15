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
  protected:
    ImagePassHandler(std::string id)
      : d_id(id)
    {}

  public:
    virtual ~ImagePassHandler() = default;

    virtual void process(ImageLabelData const& labelData, SequentialTimer& timer) = 0;

    std::string const& id() const { return d_id; };

  private:
    std::string d_id;
  };
}
