#ifndef BOLD_IMAGELABELLER_HH
#define BOLD_IMAGELABELLER_HH

#include <opencv2/core/core.hpp>

#include <iostream>
#include <vector>
#include <memory>

#include "../PixelLabel/pixellabel.hh"

namespace bold
{
  class ImageLabeller
  {
  private:
    std::shared_ptr<unsigned char> d_LUT;

  public:
    ImageLabeller(std::shared_ptr<unsigned char> const& lut);

    /** Replaces the LUT used by this image labeller. */
    void updateLut(std::shared_ptr<unsigned char> const& lut) { d_LUT = lut; }

    /**
     * Labels an entire image's pixels.
     *
     * @param image The input, colour image.
     * @param labelled The target image, in which labels are stored per-pixel.
     */
    void label(cv::Mat& image, cv::Mat& labelled) const;

    /**
     * Generates an image in which each pixel is coloured according to the label
     * assigned to it. The result looks like a cartoon, or paint-by-numbers.
     */
    static void createCartoon(cv::Mat& labelledInput, cv::Mat& cartoonOutput, std::vector<bold::PixelLabel> const& labels);
  };
}

#endif