#include "agent.ih"

#include "../Camera/camera.hh"
#include "../Debugger/debugger.hh"
#include "../DataStreamer/datastreamer.hh"
#include "../StateObject/GameState/gamestate.hh"
#include "../StateObject/BodyState/bodystate.hh"

void Agent::think()
{
  //
  // Initialise a time value that we'll repeatedly use and update to time
  // the progress of each step of the think cycle
  //
  auto t = Debugger::getTimestamp();

  Debugger& debugger = *d_debugger;

  //
  // Capture the image (YCbCr format)
  //
  cv::Mat image = d_camera->capture();
  t = debugger.timeEvent(t, "Image Capture");

  //
  // Record frame, if required
  //
  if (d_isRecordingFrames)
  {
    static Debugger::timestamp_t lastRecordTime;
    static unsigned frameNumber = 0;
    if (debugger.getSeconds(lastRecordTime) > 1.0)
    {
      // save image
      stringstream ss;
      ss << "capture-" << frameNumber++ << ".png";
      cv::imwrite(ss.str(), image);
      t = debugger.timeEvent(t, "Saving Frame To File");
      lastRecordTime = t;
    }
  }

  //
  // Process the image
  //
  d_visualCortex->integrateImage(image);
  t = debugger.timeEvent(t, "Image Processing");

  d_visualCortex->streamDebugImage(image, d_streamer);
  t = debugger.timeEvent(t, "Image Streaming");

  //
  // Listen for any game control data
  //
  shared_ptr<GameState const> gameState = d_gameStateReceiver->receive();
  if (gameState)
  {
    AgentState::getInstance().set(gameState);
    t = debugger.timeEvent(t, "Integrate Game Control");
  }

  if (d_haveBody)
  {
    if (d_useOptionTree)
    {
      d_optionTree->run();
      t = debugger.timeEvent(t, "Option Tree");
    }

    //
    // Process input commands
    //
//processInputCommands();
//t = d_debugger.timeEvent(t, "Process Human Input");

    //
    // Get up, if we've fallen over
    //
    standUpIfFallen();
    t = debugger.timeEvent(t, "Stand Up");

    //
    // Flush out new walking parameters
    //
    /*
    d_ambulator->step();
    t = debugger.timeEvent(t, "Ambulator Step");
    */
    //
    // Update LEDs on back, etc
    //
    debugger.update(d_CM730);
    t = debugger.timeEvent(t, "Update Debugger");

    //
    // Read all data from the sub board
    //
    static int tmp = 0;
    if (tmp++ % 5 == 0)
      readSubBoardData();
    t = debugger.timeEvent(t, "Read Sub Board");

    //
    // Populate agent frame from camera frame
    //
    d_spatialiser->updateCameraToAgent();
    t = debugger.timeEvent(t, "Camera to Agent Frame");

    //
    // Update the localiser
    //
    d_localiser->update();
    t = debugger.timeEvent(t, "Update Localiser");

    //
    // Populate world frame from agent frame
    //
    d_spatialiser->updateAgentToWorld(d_localiser->smoothedPosition());
    t = debugger.timeEvent(t, "Agent to World Frame");
  }

  //
  // Update websocket data
  //
  if (d_streamer != nullptr)
  {
    d_streamer->update();
    t = debugger.timeEvent(t, "Update DataStreamer");
  }

  // TODO this isn't a great approach, as the last value is lost (captured after send)
  debugger.clearTimings();
}
