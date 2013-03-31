#ifndef BOLD_IMAGEPASSRUNNER_HH
#define BOLD_IMAGEPASSRUNNER_HH

#include <opencv2/core/core.hpp>
#include <vector>

#include "../ImagePassHandler/imagepasshandler.hh"

namespace bold
{
  template <typename TPixel>
  class ImagePassRunner
  {
  public:
    ImagePassRunner(std::vector<std::shared_ptr<ImagePassHandler<TPixel>>> handlers)
    : d_handlers(handlers)
    {}

    void pass(cv::Mat& image) const
    {
      for (auto handler : d_handlers)
        handler->onImageStarting();

      for (int y = 0; y < image.rows; ++y)
      {
        TPixel const* row = image.ptr<TPixel>(y);

        for (auto handler : d_handlers)
          handler->onRowStarting(y);

        for (int x = 0; x < image.cols; ++x)
        {
          TPixel value = row[x];

          for (auto handler : d_handlers)
            handler->onPixel(value, x, y);
        }
      }

      for (auto handler : d_handlers)
        handler->onImageComplete();
    }

  private:
    std::vector<std::shared_ptr<ImagePassHandler<TPixel>>> d_handlers;

  };
}

#endif
