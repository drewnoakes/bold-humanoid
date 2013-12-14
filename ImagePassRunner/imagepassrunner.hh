#pragma once

#include <cassert>
#include <opencv2/core/core.hpp>
#include <Eigen/Core>
#include <vector>

#include "../ImagePassHandler/imagepasshandler.hh"

namespace bold
{
  template <typename TPixel>
  class ImagePassRunner
  {
  public:
    ImagePassRunner()
    : d_granularityFunction([](int i) { return Eigen::Vector2i(1,1); })
    {}

    /** Adds the specified handler, if it does not already exist in the runner.
     */
    void addHandler(std::shared_ptr<ImagePassHandler<TPixel>> handler)
    {
      if (std::find(d_handlers.begin(), d_handlers.end(), handler) == d_handlers.end())
        d_handlers.push_back(handler);
    }

    /** Removes the specified handler, if it already exists in the runner.
     */
    void removeHandler(std::shared_ptr<ImagePassHandler<TPixel>> handler)
    {
      auto it = std::find(d_handlers.begin(), d_handlers.end(), handler);

      if (it != d_handlers.end())
        d_handlers.erase(it);
    }

    /** Adds or removes the specified handler, depending upon the bool 'enabled' parameter.
     *
     * Idempotent.
     */
    void setHandler(std::shared_ptr<ImagePassHandler<TPixel>> handler, bool enabled)
    {
      if (enabled)
        addHandler(handler);
      else
        removeHandler(handler);
    }

    /** Allows processing fewer than all pixels in the image.
     *
     * Granularity is calculated based on y-value, and is specified in both
     * x and y dimensions.
     */
    void setGranularityFunction(std::function<Eigen::Vector2i(int)> function)
    {
      d_granularityFunction = function;
    }

    void pass(cv::Mat const& image) const
    {
      for (auto const& handler : d_handlers)
      {
        Eigen::Vector2i granularity(1,1);
        
        handler->onImageStarting();
        for (int y = 0; y < image.rows; y += granularity.y())
        {
          granularity = d_granularityFunction(y);

          TPixel const* row = image.ptr<TPixel>(y);

          handler->onRowStarting(y, granularity);

          int dx = granularity.x();
          for (int x = 0; x < image.cols; x += dx)
          {
            TPixel value = row[x];
            
            handler->onPixel(value, x, y);
          }
        }

        handler->onImageComplete();
      }

    }
  private:
    std::vector<std::shared_ptr<ImagePassHandler<TPixel>>> d_handlers;
    std::function<Eigen::Vector2i(int)> d_granularityFunction;
  };
}
