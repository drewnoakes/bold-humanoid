#ifndef BOLD_IMAGELABELLER_HH
#define BOLD_IMAGELABELLER_HH

#include <opencv2/core/core.hpp>

#include <iostream>
#include <vector>

#include "../LUTBuilder/lutbuilder.hh"

namespace bold
{
  class ImageLabeller
  {
  private:
    unsigned char const * const d_LUT;

  public:
    ImageLabeller(std::vector<bold::hsvRange> ranges);

    void label(cv::Mat& image, cv::Mat& labelled);

    static void colourLabels(cv::Mat& labelledImage, cv::Mat& output, std::vector<hsvRange> const& ranges);
  };
}

#endif