#include "agent.ih"

void Agent::think()
{
//   cout << "[Agent::think]" << endl;

  SequentialTimer t;

  //
  // Capture the image (YCbCr format)
  //
//   t->enter("Image Capture");
  cv::Mat image = d_camera->capture(t);
//   t->exit();
  t.timeEvent("Image Capture");

  //
  // Record frame, if required
  //
  if (d_isRecordingFrames)
  {
    static Clock::Timestamp lastRecordTime;
    static unsigned frameNumber = 0;
    if (Clock::getSecondsSince(lastRecordTime) > 1.0)
    {
      // save image
      stringstream ss;
      ss << "capture-" << frameNumber++ << ".png";
      cv::imwrite(ss.str(), image);
      t.timeEvent("Saving Frame To File");
      lastRecordTime = Clock::getTimestamp();
    }
  }

  //
  // Process the image
  //
//   t->enter("Image Processing");
  d_visualCortex->integrateImage(image, t);
//   t->exit();
  t.timeEvent("Image Processing");

//   t->enter("Image Streaming");
  d_visualCortex->streamDebugImage(image, d_streamer, t);
//   t->exit();
  t.timeEvent("Image Streaming");

  //
  // Listen for any game control data
  //
  shared_ptr<GameState const> gameState = d_gameStateReceiver->receive();
  if (gameState)
  {
    AgentState::getInstance().set(gameState);
    t.timeEvent("Integrate Game Control");
  }

  //
  // Populate agent frame from camera frame
  //
  d_spatialiser->updateCameraToAgent();
  t.timeEvent("Camera to Agent Frame");

  //
  // Update the localiser
  //
  d_localiser->update();
  t.timeEvent("Update Localiser");

  //
  // Populate world frame from agent frame
  //
  d_spatialiser->updateAgentToWorld(d_localiser->smoothedPosition());
  t.timeEvent("Agent to World Frame");

  if (d_haveBody)
  {
    if (d_useOptionTree)
    {
      d_optionTree->run();
      t.timeEvent("Option Tree");
    }

    //
    // Process input commands
    //
    processInputCommands();
    t.timeEvent("Process Human Input");

    //
    // Get up, if we've fallen over
    //
    // TODO make this a behaviour
    standUpIfFallen();
    t.timeEvent("Stand Up");

    //
    // Flush out new walking parameters
    //
    // TODO this becomes part of the motion loop
    d_ambulator->step();
    t.timeEvent("Ambulator Step");

    //
    // Update LEDs on back, etc
    //
    d_debugger->update(d_cm730);
    t.timeEvent("Update Debugger");
  }

  //
  // Update websocket data
  //
  if (d_streamer != nullptr)
  {
    d_streamer->update();
    t.timeEvent("Update DataStreamer");
  }

  //
  // Set timing data for the think cycle
  //
  AgentState::getInstance().set(make_shared<ThinkTimingState const>(t.flush()));

  onThinkEnd();
}
