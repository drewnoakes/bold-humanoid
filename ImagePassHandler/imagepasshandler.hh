#pragma once

#include <Eigen/Core>

namespace bold
{
  typedef unsigned char uchar;
  typedef unsigned short int ushort;

  /**
   * Abstract base class for classes that process the pixels of an image.
   *
   * {@link ImagePassRunner} accepts one or more instances of this type, enabling
   * multiple processes to feed from a single pass of the input image's pixels.
   */
  template <typename TPixel>
  class ImagePassHandler
  {
  public:
    virtual ~ImagePassHandler() {}

    /** Processing of an image frame is about to begin. */
    virtual void onImageStarting() {}

    /** The row 'y' is about to start. */
    virtual void onRowStarting(ushort y, Eigen::Vector2i const& granularity) {}

    /**
     * The pixel at 'x', 'y', has a label of 'labelId'.
     * This is the only abstract function on this class.
     */
    virtual void onPixel(TPixel labelId, ushort x, ushort y) = 0;

    /** Processing of an image frame has completed. */
    virtual void onImageComplete() {}
  };
}
