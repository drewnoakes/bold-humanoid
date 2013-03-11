#include "datastreamer.ih"

bool DataStreamer::shouldProvideImage()
{
  // Current logic is just to provide an image every N cycles.
  // With experience we may find a better solution.

  return AgentModel::getInstance().getCycleNumber() % d_streamFramePeriod == 0;
}