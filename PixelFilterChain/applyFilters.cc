#include "pixelfilterchain.ih"

void PixelFilterChain::applyFilters(cv::Mat& img) const
{
  uchar* imgCursor = img.data;
  uchar* imgEnd = img.data + 3 * img.rows * img.cols;

  while (imgCursor < imgEnd)
  {
    for (auto filter : d_filters)
      filter(imgCursor);

    imgCursor++;
    imgCursor++;
    imgCursor++;
  }
}
