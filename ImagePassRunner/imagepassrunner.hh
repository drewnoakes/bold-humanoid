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

    /** Passes over the image, calling out to all ImagePassHandlers with data from the image.
     */
    void pass(ImageLabelData const& labelData, SequentialTimer& timer) const
    {
      ASSERT(d_handlers.size());

      for (auto const& handler : d_handlers)
      {
        timer.enter(handler->id());
        handler->process(labelData, timer);
        timer.exit();
      }
    }

    std::set<std::shared_ptr<ImagePassHandler<TPixel>>> d_handlers;
  };
}
