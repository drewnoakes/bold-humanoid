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
#include "../ImageLabelData/imagelabeldata.hh"

namespace bold
{
  template <typename TPixel>
  class ImagePassRunner
  {
  public:
    ImagePassRunner() = default;

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
     */
    void pass(ImageLabelData const& labelData, SequentialTimer& timer) const
    {
      ASSERT(d_handlers.size());

      for (auto const& handler : d_handlers)
      {
        timer.enter(handler->id());

        handler->onImageStarting(timer);

        for (auto const& row : labelData.getRows())
        {
          handler->onRowStarting(row.imageY, row.granularity);

          ushort x = 0;
          for (auto const& label : row.labels)
          {
            handler->onPixel(label, x, row.imageY);
            x += row.granularity.x();
          }

          handler->onRowCompleted(row.imageY, row.granularity);
        }

        timer.timeEvent("Pass");

        handler->onImageComplete(timer);

        timer.exit();
      }
    }

    /** Passes over the image with the given handler.
     *
     * This method bypasses vtable lookups, so the handler must be
     * typed correctly; it can't be a pointer to a parent type. A
     * granularity function allows processing fewer than all pixels in
     * the image.
     */
    template<typename T>
    void passWithHandler(std::shared_ptr<T> handler, ImageLabelData const& labelData, SequentialTimer& timer) const
    {
      ASSERT(handler);

      timer.enter(handler->id());

      handler->T::onImageStarting(timer);

      for (auto const& row : labelData.getRows())
      {
        handler->T::onRowStarting(row.imageY, row.granularity);

        ushort x = 0;
        for (auto const& label : row.labels)
        {
          handler->T::onPixel(label, x, row.imageY);
          x += row.granularity.x();
        }

        handler->T::onRowCompleted(row.imageY, row.granularity);
      }

      timer.timeEvent("Pass");

      handler->T::onImageComplete(timer);

      timer.exit();
    }

    /** Passes over the image, calling out to all ImagePassHandlers in
     * the given tuple with data from the image.
     */

    template<typename... Types>
    void passWithHandlers(std::tuple<Types...> const& handlers,
                          ImageLabelData const& labelData,
                          SequentialTimer& timer) const
    {
      meta::for_each<PassWrapper>(handlers, this, labelData, timer);
    }

  private:
    struct PassWrapper
    {
      template<typename Handler>
      static void do_it(std::shared_ptr<Handler> handler,
                        ImagePassRunner const* runner,
                        ImageLabelData const& labelData,
                        SequentialTimer& timer)
      {
        // TODO: disabling this makes direct pass faster??
        // In any case good to prevent this check at every pass again
        if (runner->isEnabled(handler))
          runner->passWithHandler(handler, labelData, timer);
      }
    };

    std::set<std::shared_ptr<ImagePassHandler<TPixel>>> d_handlers;
  };
}
