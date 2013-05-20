#pragma once

namespace bold
{
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

    /** Image processing is about to begin. */
    virtual void onImageStarting() {}

    /** Image processing has completed. */
    virtual void onImageComplete() {}

    /** The row 'y' is about to start. */
    virtual void onRowStarting(ushort y) {}

    /**
     * The pixel at 'x', 'y', has a value of 'value'.
     * This is the only abstract function on this class.
     */
    virtual void onPixel(TPixel value, ushort x, ushort y) = 0;
  };
}
