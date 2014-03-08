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
  shared_ptr<GameState const> gameState = d_gameStateReceiver->receive();
  if (gameState)
    State::set(gameState);
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

  d_optionTree->run();
  t.timeEvent("Option Tree");
  
  //
  // Attempt to recieve from other agents
  //
  d_openTeamCommunicator->recieveData();
  t.timeEvent("Open Team Recieve");
  
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
  State::set(make_shared<ThinkTimingState const>(t.flush(), d_cycleNumber));

  onThinkEnd();

  t.enter("Observers");
  State::callbackObservers(ThreadId::ThinkLoop, t);
  t.exit();

  log::verbose("Agent::think") << "Ending think cycle " << d_cycleNumber;
}
