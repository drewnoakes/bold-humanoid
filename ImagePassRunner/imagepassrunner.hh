#pragma once

#include <functional>
#include <opencv2/core/core.hpp>
#include <Eigen/Core>
#include <vector>
#include <set>

#include "../ImagePassHandler/imagepasshandler.hh"
#include "../ImageSampleMap/imagesamplemap.hh"
#include "../SequentialTimer/sequentialtimer.hh"
#include "../util/assert.hh"
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
    void pass(cv::Mat const& image,
              ImageSampleMap const& sampleMap,
              SequentialTimer& timer) const
    {
      ASSERT(image.rows);
      ASSERT(image.cols);
      ASSERT(d_handlers.size());

      for (auto const& handler : d_handlers)
      {
        timer.enter(handler->id());

        auto granularity = sampleMap.begin();

        handler->onImageStarting(timer);

        for (ushort y = 0; y < image.rows; y += granularity->y(), granularity++)
        {
          TPixel const* row = image.ptr<TPixel>(y);

          handler->onRowStarting(y, *granularity);

          int dx = granularity->x();
          for (int x = 0; x < image.cols; x += dx)
          {
            TPixel value = row[x];

            handler->onPixel(value, x, y);
          }

          handler->onRowCompleted(y, *granularity);
        }

        timer.timeEvent("Pass");

        handler->onImageComplete(timer);

        timer.exit();
      }
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
    void passWithHandler(std::shared_ptr<T> handler,
                         cv::Mat const& image,
                         ImageSampleMap const& sampleMap,
                         SequentialTimer& timer) const
    {
      ASSERT(image.rows);
      ASSERT(image.cols);
      ASSERT(handler);

      timer.enter(handler->id());

      auto granularity = sampleMap.begin();

      handler->T::onImageStarting(timer);

      for (int y = 0; y < image.rows; y += granularity->y(), granularity++)
      {
        TPixel const* row = image.ptr<TPixel>(y);

        handler->T::onRowStarting(y, *granularity);

        int dx = granularity->x();
        for (int x = 0; x < image.cols; x += dx)
        {
          TPixel value = row[x];

          handler->T::onPixel(value, x, y);
        }

        handler->T::onRowCompleted(y, *granularity);
      }

      timer.timeEvent("Pass");

      handler->T::onImageComplete(timer);

      timer.exit();
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
    void passWithHandlers(std::tuple<Types...> const& handlers,
                          cv::Mat const& image,
                          ImageSampleMap const& sampleMap,
                          SequentialTimer& timer) const
    {
      meta::for_each<PassWrapper>(handlers, this, image, sampleMap, timer);
    }

  private:
    struct PassWrapper
    {
      template<typename Handler>
      static void do_it(std::shared_ptr<Handler> handler,
                        ImagePassRunner const* runner,
                        cv::Mat const& image,
                        ImageSampleMap const& sampleMap,
                        SequentialTimer& timer)
      {
        // TODO: disabling this makes direct pass faster??
        // In any case good to prevent this check at every pass again
        if (runner->isEnabled(handler))
          runner->passWithHandler(handler, image, sampleMap, timer);
      }
    };

    std::set<std::shared_ptr<ImagePassHandler<TPixel>>> d_handlers;
  };

}

