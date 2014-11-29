#pragma once

#include <set>
#include <memory>
#include <mutex>

namespace bold
{
  class ImageLabelData;
  class ImagePassHandler;
  class SequentialTimer;

  class ImagePassRunner
  {
  public:
    ImagePassRunner() = default;

    /** Adds the specified handler, if it does not already exist in the runner.
     */
    void addHandler(std::shared_ptr<ImagePassHandler> handler);

    /** Removes the specified handler, if it already exists in the runner.
     */
    void removeHandler(std::shared_ptr<ImagePassHandler> handler);

    /** Adds or removes the specified handler, depending upon the bool 'enabled' parameter.
     *
     * Idempotent.
     */
    void setHandler(std::shared_ptr<ImagePassHandler> handler, bool enabled);

    /** Passes over the image, calling out to all ImagePassHandlers with data from the image.
     */
    void pass(ImageLabelData const& labelData, SequentialTimer& timer) const;

    std::set<std::shared_ptr<ImagePassHandler>> d_handlers;

  private:
    mutable std::mutex d_handlerMutex;
  };
}
