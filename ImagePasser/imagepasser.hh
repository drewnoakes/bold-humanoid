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
        TPixel const* row = image.ptr<TPixel>(y);

        for (ImagePassHandler<TPixel>* handler : d_handlers)
          handler->onRowStarting(y);

        for (int x = 0; x < image.cols; ++x)
        {
          TPixel value = row[x];

          for (ImagePassHandler<TPixel>* handler : d_handlers)
            handler->onPixel(value, x, y);
        }
      }

      for (ImagePassHandler<TPixel>* handler : d_handlers)
        handler->onImageComplete();
    }
  };
}

#endif