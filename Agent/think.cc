#include "agent.ih"

void Agent::think()
{
  //
  // Print out time since last entry to 'think'
  //
  static auto tLast = Debugger::getTimestamp();
  Debugger::printTime(tLast, "Period %4.2f ");
  tLast = Debugger::getTimestamp();

  //
  // Capture the image
  //
  auto t = Debugger::getTimestamp();
  cv::Mat raw;
  d_camera >> raw;
  d_debugger.timeImageCapture(t);

  //
  // Process the image
  //
  t = Debugger::getTimestamp();
  d_observations = processImage(raw);
  d_debugger.timeImageProcessing(t);

  cout << "state: " << d_state << endl;

  switch (d_state)
  {
  case S_INIT:
    d_state = S_LOOK_FOR_BALL;

  case S_LOOK_FOR_BALL:
    lookForBall();
    break;

  case S_APPROACH_BALL:
    approachBall();
    break;

  case S_LOOK_FOR_GOAL:
    lookForGoal();
    break;

  case S_START_CIRCLE_BALL:
  case S_CIRCLE_BALL:
    circleBall();
    break;

  case S_START_PREKICK_LOOK:
  case S_PREKICK_LOOK:
    preKickLook();
    break;

  case S_KICK:
    kick();
    stand();
  
    break;

  case S_GET_UP:
    getUp();
    break;
  }

  /*
  //
  // Track ball position with head
  //
  controlHead(raw, observations);

  //
  // Get up, if we've fallen over
  //
  standUpIfFallen();

  //
  // Process input commands
  //
  processInputCommands();

  */

  d_ambulator.step();

  d_debugger.update(d_CM730);
}
