#ifndef BOLD_IMAGE_PASSER_HH
#define BOLD_IMAGE_PASSER_HH

#include <opencv2/core/core.hpp>
#include <vector>

#include "../ImagePassHandler/imagepasshandler.hh"

namespace bold
{
  template <typename TPixel>
  class ImagePasser
  {
  private:
    std::vector<ImagePassHandler<TPixel>*> d_handlers;

  public:
    ImagePasser(std::vector<ImagePassHandler<TPixel>*> handlers)
    : d_handlers(handlers)
    {}

    void pass(cv::Mat& image)
    {
      for (ImagePassHandler<TPixel>* handler : d_handlers)
        handler->onImageStarting();

      for (int y = 0; y < image.rows; ++y)
      {
        uchar const* row = image.ptr<uchar>(y);

        for (ImagePassHandler<TPixel>* handler : d_handlers)
          handler->onRowStarting(y);

        // We go one pixel outside of the row, as if image is padded with a column of zeros
        for (int x = 0; x != image.cols; ++x)
        {
          uchar label = x == image.cols ? 0 : row[x];

          for (ImagePassHandler<TPixel>* handler : d_handlers)
            handler->onPixel(label, x, y);
        }
      }

      for (ImagePassHandler<TPixel>* handler : d_handlers)
        handler->onImageComplete();
    }
  };
}

#endif