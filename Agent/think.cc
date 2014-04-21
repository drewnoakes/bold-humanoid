#include "agent.ih"

void Agent::think()
{
  ASSERT(ThreadUtil::isThinkLoopThread());

  d_cycleNumber++;

  log::verbose("Agent::think") << "Starting think cycle " << d_cycleNumber;

  SequentialTimer t;

  //
  // Capture the image (YCbCr format)
  // This should be the very first thing done in the think loop, to
  // ensure a BodyState snapshot is available
  //
  t.enter("Image Capture");
  cv::Mat image = d_camera->capture(t);
  t.exit();

  //
  // Update Spatialiser internals
  //
  d_spatialiser->updateZeroGroundPixelTransform();

  //
  // Process the image
  //
  t.enter("Vision");
  d_visualCortex->integrateImage(image, t, d_cycleNumber);
  t.exit();

  d_visualCortex->streamDebugImage(image, t);

  //
  // Listen for any game control data
  //
  d_gameStateReceiver->receive();
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

  //
  // Attempt to receive from other agents
  //
  d_teamCommunicator->receiveData();
  t.timeEvent("Team Receive");

  //
  // Decide role, updating BehaviourControl
  //
  d_roleDecider->update();
  t.timeEvent("Role Decision");

  //
  // Run the option tree to execute behaviour
  //
  d_optionTree->run();
  t.timeEvent("Option Tree");

  //
  // Process remote control (eg. joystick)
  //
  d_remoteControl->update();
  t.timeEvent("Remote Control");

  //
  // Update LEDs on back, etc
  //
  d_debugger->update();
  t.timeEvent("Update Debugger");

  // Refresh MotionTaskState, if one is needed
  d_motionSchedule->update();
  t.timeEvent("Update Motion Schedule");

  //
  // Set timing data for the think cycle
  //
  State::make<ThinkTimingState>(t.flush(), d_cycleNumber);

  //
  // Raise the Agent::onThinkEnd signal
  //
  onThinkEnd();

  //
  // Invoke observers which requested to be called back from the think loop
  //
  t.enter("Observers");
  State::callbackObservers(ThreadId::ThinkLoop, t);
  t.exit();

  //
  // Send a message for drawbridge use within matches, containing status of the agent.
  //
  if (d_cycleNumber % 30 == 0)
    d_drawBridgeComms->publish();

  log::verbose("Agent::think") << "Ending think cycle " << d_cycleNumber;
}
