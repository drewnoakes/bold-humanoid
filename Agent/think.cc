#include "agent.ih"

void Agent::think()
{
  cout << "[Agent::think]" << endl;

  //
  // Initialise a time value that we'll repeatedly use and update to time
  // the progress of each step of the think cycle
  //
  auto t = Clock::getTimestamp();

  //
  // Capture the image (YCbCr format)
  //
  cv::Mat image = d_camera->capture();
  t = d_debugger->timeEvent(t, "Image Capture");

  //
  // Record frame, if required
  //
  if (d_isRecordingFrames)
  {
    static Clock::Timestamp lastRecordTime;
    static unsigned frameNumber = 0;
    if (Clock::getSeconds(lastRecordTime) > 1.0)
    {
      // save image
      stringstream ss;
      ss << "capture-" << frameNumber++ << ".png";
      cv::imwrite(ss.str(), image);
      t = d_debugger->timeEvent(t, "Saving Frame To File");
      lastRecordTime = t;
    }
  }

  //
  // Process the image
  //
  d_visualCortex->integrateImage(image);
  t = d_debugger->timeEvent(t, "Image Processing");

  d_visualCortex->streamDebugImage(image, d_streamer);
  t = d_debugger->timeEvent(t, "Image Streaming");

  //
  // Listen for any game control data
  //
  shared_ptr<GameState const> gameState = d_gameStateReceiver->receive();
  if (gameState)
  {
    AgentState::getInstance().set(gameState);
    t = d_debugger->timeEvent(t, "Integrate Game Control");
  }

  //
  // Populate agent frame from camera frame
  //
  d_spatialiser->updateCameraToAgent();
  t = d_debugger->timeEvent(t, "Camera to Agent Frame");

  //
  // Update the localiser
  //
  d_localiser->update();
  t = d_debugger->timeEvent(t, "Update Localiser");

  //
  // Populate world frame from agent frame
  //
  d_spatialiser->updateAgentToWorld(d_localiser->smoothedPosition());
  t = d_debugger->timeEvent(t, "Agent to World Frame");

  if (d_haveBody)
  {
    if (d_useOptionTree)
    {
      d_optionTree->run();
      t = d_debugger->timeEvent(t, "Option Tree");
    }

    //
    // Process input commands
    //
    processInputCommands();
    t = d_debugger->timeEvent(t, "Process Human Input");

    //
    // Get up, if we've fallen over
    //
    // TODO make this a behaviour
    standUpIfFallen();
    t = d_debugger->timeEvent(t, "Stand Up");

    //
    // Flush out new walking parameters
    //
    // TODO this becomes part of the motion loop
    d_ambulator->step();
    t = d_debugger->timeEvent(t, "Ambulator Step");

    //
    // Update LEDs on back, etc
    //
    d_debugger->update(d_cm730);
    t = d_debugger->timeEvent(t, "Update Debugger");
  }

  //
  // Update websocket data
  //
  if (d_streamer != nullptr)
  {
    d_streamer->update();
    // NOTE this timing value will appear in the *next* think cycle
    t = d_debugger->timeEvent(t, "Update DataStreamer");
  }

  onThinkEnd();
}
