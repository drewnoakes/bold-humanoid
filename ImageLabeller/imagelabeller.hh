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
    ImageLabeller(std::vector<bold::hsvRange> ranges)
    : d_LUT(LUTBuilder().buildBGR18FromHSVRanges(ranges))
    {
      std::cout << "[ImageLabeller::ImageLabeller] Constructed" << std::endl;
    }

    /**
     *
     */
    void label(cv::Mat& image, cv::Mat& labelled)
    {
      for (unsigned y = 0; y < image.rows; ++y)
      {
        unsigned char *origpix = image.ptr<unsigned char>(y);
        unsigned char *labelledpix = labelled.ptr<unsigned char>(y);
        for (unsigned x = 0; x < image.cols; ++x)
        {
//        unsigned char l = d_LUT[(origpix[0] << 16) | (origpix[1] << 8) | origpix[2]];
          unsigned char l = d_LUT[((origpix[0] >> 2) << 12) | ((origpix[1] >> 2) << 6) | (origpix[2] >> 2)];
          *labelledpix = l;

          ++origpix;
          ++origpix;
          ++origpix;
          ++labelledpix;
        }
      }
    }

    static void colourLabels(cv::Mat& labelledImage, cv::Mat& output)
    {
      // TODO implement ImageLabeller::colourLabels
    }
  };
}

#endif