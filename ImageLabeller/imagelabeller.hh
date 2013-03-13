#ifndef BOLD_IMAGELABELLER_HH
#define BOLD_IMAGELABELLER_HH

#include <opencv2/core/core.hpp>

#include <iostream>
#include <vector>

#include "../PixelLabel/pixellabel.hh"

namespace bold
{
  class ImageLabeller
  {
  private:
    uchar const * const d_LUT;

  public:
    ImageLabeller(uchar const * const lut);

    /**
     * Labels an entire image's pixels.
     *
     * @param image The input, colour image.
     * @param labelled The target image, in which labels are stored per-pixel.
     */
    void label(cv::Mat& image, cv::Mat& labelled);

    /**
     * Generates an image in which each pixel is coloured according to the label
     * assigned to it. The result looks like a cartoon, or paint-by-numbers.
     */
    static void colourLabels(cv::Mat& labelledImage, cv::Mat& output, std::vector<bold::PixelLabel> const& labels);
  };
}

#endif