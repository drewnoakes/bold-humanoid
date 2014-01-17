#include "agent.ih"

void Agent::think()
{
  assert(ThreadId::isThinkLoopThread());

  d_cycleNumber++;

  log::verbose("Agent::think") << "Starting think cycle " << d_cycleNumber;

  SequentialTimer t;

  //
  // Capture the image (YCbCr format)
  //
  t.enter("Image Capture");
  cv::Mat image = d_camera->capture(t);
  t.exit();

  //
  // Update BodyState
  //
  // BodyState is updated immediately after capturing the camera image so that
  // it's as close to the state of the body at the time of capture as possible.
  // This means that throughout each think cycle, the body state doesn't change.
  //

  auto hw = AgentState::get<HardwareState>();
  if (hw)
  {
    double angles[(uchar)JointId::MAX + 1];
    for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      angles[jointId] = hw->getMX28State(jointId)->presentPosition;

    AgentState::getInstance().set(make_shared<BodyState const>(angles, d_cycleNumber));
    t.timeEvent("Update BodyState");
  }

  //
  // Process the image
  //
  t.enter("Image Processing");
  d_visualCortex->integrateImage(image, t);
  t.exit();

  t.enter("Image Streaming");
  d_visualCortex->streamDebugImage(image, d_streamer, t);
  t.exit();

  //
  // Listen for any game control data
  //
  shared_ptr<GameState const> gameState = d_gameStateReceiver->receive();
  if (gameState)
    AgentState::getInstance().set(gameState);
  t.timeEvent("Integrate Game Control");

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

  // TODO allow these updates when we don't have a body attached, for better debugging on non-robot machines

  if (d_haveBody)
  {
    d_optionTree->run();
    t.timeEvent("Option Tree");

    //
    // Process input commands (joystick)
    //
    processInputCommands();
    t.timeEvent("Process Human Input");

    //
    // Flush out new walking parameters
    //
    // TODO this becomes part of the motion loop for smoother control
    d_ambulator->step();
    t.timeEvent("Ambulator Step");

    //
    // Update LEDs on back, etc
    //
    d_debugger->update();
    t.timeEvent("Update Debugger");
  }

  // Refresh MotionTaskState, if one is needed
  d_motionSchedule->update();
  t.timeEvent("Update Motion Schedule");

  //
  // Set timing data for the think cycle
  //
  AgentState::getInstance().set(make_shared<ThinkTimingState const>(t.flush(), d_cycleNumber));

  onThinkEnd();

  t.enter("Observers");
  AgentState::getInstance().callbackObservers(ThreadIds::ThinkLoop, t);
  t.exit();

  log::verbose("Agent::think") << "Ending think cycle " << d_cycleNumber;
}
