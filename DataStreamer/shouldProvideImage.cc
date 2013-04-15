#include "datastreamer.ih"

#include "../StateObject/CameraFrameState/cameraframestate.hh"

bool DataStreamer::shouldProvideImage()
{
  // Current logic is just to provide an image every N cycles.
  // With experience we may find a better solution.

  return d_cameraSessions.size() != 0
      && AgentState::getInstance().getTracker<CameraFrameState>()->updateCount() % d_streamFramePeriod == 0;
}