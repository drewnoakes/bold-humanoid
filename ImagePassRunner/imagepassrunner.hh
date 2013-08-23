#pragma once

#include <cassert>
#include <opencv2/core/core.hpp>
#include <vector>

#include "../ImagePassHandler/imagepasshandler.hh"

namespace bold
{
  template <typename TPixel>
  class ImagePassRunner
  {
  public:
    void addHandler(std::shared_ptr<ImagePassHandler<TPixel>> handler)
    {
      if (std::find(d_handlers.begin(), d_handlers.end(), handler) == d_handlers.end())
        d_handlers.push_back(handler);
    }

    void removeHandler(std::shared_ptr<ImagePassHandler<TPixel>> handler)
    {
      auto it = std::find(d_handlers.begin(), d_handlers.end(), handler);

      if (it != d_handlers.end())
        d_handlers.erase(it);
    }

    void pass(cv::Mat& image) const
    {
      for (auto const& handler : d_handlers)
        handler->onImageStarting();

      for (int y = 0; y < image.rows; ++y)
      {
        TPixel const* row = image.ptr<TPixel>(y);

        for (auto const& handler : d_handlers)
          handler->onRowStarting(y);

        for (int x = 0; x < image.cols; ++x)
        {
          TPixel value = row[x];

          for (auto const& handler : d_handlers)
            handler->onPixel(value, x, y);
        }
      }

      for (auto const& handler : d_handlers)
        handler->onImageComplete();
    }

  private:
    std::vector<std::shared_ptr<ImagePassHandler<TPixel>>> d_handlers;
  };
}
