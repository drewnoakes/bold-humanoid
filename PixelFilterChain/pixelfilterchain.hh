#ifndef BOLD_PIXELFITLERCHAIN_HH
#define BOLD_PIXELFILTERCHAIN_HH

#include <functional>
#include <opencv2/opencv.hpp>

namespace bold
{
  class PixelFilterChain
  {
  public:
    PixelFilterChain()
    {}

    void pushFilter(std::function<void(unsigned char*)> const& filter)
    {
      d_filters.push_back(filter);
    }

    void applyFilters(cv::Mat& img) const;

  private:
    std::vector<std::function<void(unsigned char*)>> d_filters;
  };
}

#endif
