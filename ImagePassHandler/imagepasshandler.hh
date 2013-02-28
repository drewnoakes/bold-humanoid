#ifndef BOLD_IMAGE_PASS_HANDLER_HH
#define BOLD_IMAGE_PASS_HANDLER_HH

namespace bold
{
  /**
   * Abstract base class for classes that process the pixels of an image.
   *
   * {@link ImagePasser} accepts one or more instances of this type, enabling
   * multiple processes to feed from a single pass of the input image's pixels.
   */
  template <typename TPixel>
  class ImagePassHandler
  {
  public:
    /** Image processing is about to begin. */
    virtual void onImageStarting() {}

    /** Image processing has completed. */
    virtual void onImageComplete() {}

    /** The row 'y' is about to start. */
    virtual void onRowStarting(int y) {}

    /**
     * The pixel at 'x', 'y', has a value of 'value'.
     * This is the only abstract function on this class.
     */
    virtual void onPixel(TPixel value, int x, int y) = 0;
  };
}

#endif