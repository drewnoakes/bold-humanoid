#include "imagepassrunner.hh"

#include "../ImagePassHandler/imagepasshandler.hh"
#include "../ImageSampleMap/imagesamplemap.hh"
#include "../ImageLabelData/imagelabeldata.hh"
#include "../SequentialTimer/sequentialtimer.hh"

using namespace bold;
using namespace std;

void ImagePassRunner::addHandler(std::shared_ptr<ImagePassHandler> handler)
{
  if (std::find(d_handlers.begin(), d_handlers.end(), handler) == d_handlers.end())
    d_handlers.insert(handler);
}

/** Removes the specified handler, if it already exists in the runner.
*/
void ImagePassRunner::removeHandler(std::shared_ptr<ImagePassHandler> handler)
{
  auto it = d_handlers.find(handler);

  if (it != d_handlers.end())
    d_handlers.erase(it);
}

/** Adds or removes the specified handler, depending upon the bool 'enabled' parameter.
*
* Idempotent.
*/
void ImagePassRunner::setHandler(std::shared_ptr<ImagePassHandler> handler, bool enabled)
{
  if (enabled)
    addHandler(handler);
  else
    removeHandler(handler);
}

/** Passes over the image, calling out to all ImagePassHandlers with data from the image.
*/
void ImagePassRunner::pass(ImageLabelData const& labelData, SequentialTimer& timer) const
{
  for (auto const& handler : d_handlers)
  {
    timer.enter(handler->id());
    handler->process(labelData, timer);
    timer.exit();
  }
}
