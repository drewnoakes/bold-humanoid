#pragma once

#include <cassert>
#include <functional>
#include <opencv2/core/core.hpp>
#include <Eigen/Core>
#include <vector>
#include <set>

#include "../ImagePassHandler/imagepasshandler.hh"
#include "../util/meta.hh"

namespace bold
{
  template <typename TPixel>
  class ImagePassRunner
  {
  public:
    ImagePassRunner()
    {}

    /** Adds the specified handler, if it does not already exist in the runner.
     */
    void addHandler(std::shared_ptr<ImagePassHandler<TPixel>> handler)
    {
      if (std::find(d_handlers.begin(), d_handlers.end(), handler) == d_handlers.end())
        d_handlers.insert(handler);
    }

    /** Removes the specified handler, if it already exists in the runner.
     */
    void removeHandler(std::shared_ptr<ImagePassHandler<TPixel>> handler)
    {
      auto it = d_handlers.find(handler);

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

    /** Check whether the specified handler is enabled
     */
    bool isEnabled(std::shared_ptr<ImagePassHandler<TPixel>> handler) const
    {
      return d_handlers.find(handler) != d_handlers.end();
    }

    /** Passes over the image, calling out to all ImagePassHandlers
     * with data from the image.
     *
     * A granularity function allows processing fewer than all pixels
     * in the image.
     *
     * Granularity is calculated based on y-value, and is specified in
     * both x and y dimensions.
     *
     * @param image Labelled image data
     * @param granularityFunction The function used to determine
     * granularity within the image.
     */
    long pass(cv::Mat const& image, std::function<Eigen::Vector2i(int)> const& granularityFunction) const
    {
      assert(image.rows);
      assert(image.cols);
      assert(d_handlers.size());

      long pixelCount = 0;

      for (auto const& handler : d_handlers)
      {
        Eigen::Vector2i granularity;

        handler->onImageStarting();
        
        for (int y = 0; y < image.rows; y += granularity.y())
        {
          granularity = granularityFunction(y);
          
          pixelCount += image.cols / granularity.x();
          
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
      return pixelCount / d_handlers.size();
    }

    /** Passes over the image with the given handler
     *
     * This method bypasses vtable lookups, so the handler must be
     * typed correctly; it can't be a pointer to a parent type. A
     * granularity function allows processing fewer than all pixels in
     * the image.
     *
     * Granularity is calculated based on y-value, and is specified in
     * both x and y dimensions.
     *
     * @param image Labelled image data
     * @param granularityFunction The function used to determine
     * granularity within the image.
     */
    template<typename T>
    long passWithHandler(std::shared_ptr<T> handler,
                         cv::Mat const& image, std::function<Eigen::Vector2i(int)> const& granularityFunction) const
    {
      assert(image.rows);
      assert(image.cols);
      assert(handler);

      long pixelCount = 0;
      Eigen::Vector2i granularity;

      handler->T::onImageStarting();
      
      for (int y = 0; y < image.rows; y += granularity.y())
      {
        granularity = granularityFunction(y);
        
        pixelCount += image.cols / granularity.x();
        
        TPixel const* row = image.ptr<TPixel>(y);
        
        handler->T::onRowStarting(y, granularity);
        
        int dx = granularity.x();
        for (int x = 0; x < image.cols; x += dx)
        {
          TPixel value = row[x];
          
          handler->T::onPixel(value, x, y);
        }
      }
      
      handler->T::onImageComplete();
      return pixelCount;
    }

    /** Passes over the image, calling out to all ImagePassHandlers in
     * the given tuple with data from the image.
     *
     * A granularity function allows processing fewer than all pixels
     * in the image.
     *
     * Granularity is calculated based on y-value, and is specified in
     * both x and y dimensions.
     *
     * @param image Labelled image data
     * @param granularityFunction The function used to determine
     * granularity within the image.
     */

    template<typename... Types>
    long passWithHandlers(std::tuple<Types...> const& handlers,
                          cv::Mat const& image,
                          std::function<Eigen::Vector2i(int)> const& granularityFunction) const
    {
      long pixelCount = 0;
      meta::for_each<PassWrapper>(handlers, this, pixelCount, image, granularityFunction);
      return pixelCount;
    }

  private:
    struct PassWrapper
    {
      template<typename Handler>
      static void do_it(std::shared_ptr<Handler> handler,
                        ImagePassRunner const* runner,
                        long& pixelCount,
                        cv::Mat const& image,
                        std::function<Eigen::Vector2i(int)> const& granularityFunction)
      {
        // TODO: disabling this makes direct pass faster??
        // In any case good to prevent this check at every pass again
        if (runner->isEnabled(handler))
          pixelCount += runner->passWithHandler(handler, image, granularityFunction);
      }
    };

    std::set<std::shared_ptr<ImagePassHandler<TPixel>>> d_handlers;
  };

}

