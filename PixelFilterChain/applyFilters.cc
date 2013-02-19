#include "pixelfilterchain.ih"

void PixelFilterChain::applyFilters(cv::Mat& img)
{
  unsigned char* imgCursor = img.data;
  unsigned char* imgEnd = img.data + 3 * img.rows * img.cols;
  while (imgCursor < imgEnd)
  {
    for (auto filter : d_filters)
      filter(imgCursor);
    imgCursor ++;
    imgCursor ++;
    imgCursor ++;
  }
  
}
