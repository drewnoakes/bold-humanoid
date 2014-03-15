#include "agent.ih"

void Agent::think()
{
  assert(ThreadUtil::isThinkLoopThread());

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
  // Process input commands (joystick)
  //
  processInputCommands();
  t.timeEvent("Process Human Input");

  //
  // Flush out new walking parameters
  //
  // TODO do this in the motion loop for smoother interpolation
  d_ambulator->step();
  t.timeEvent("Ambulator Step");

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


  log::verbose("Agent::think") << "Ending think cycle " << d_cycleNumber;
}
