#include "agent.ih"
#include "../AgentModel/agentmodel.hh"
#include "../GameController/RoboCupGameControlData.h"

void Agent::think()
{
  //
  // Print out time since last entry to 'think'
  //
  static auto tLast = Debugger::getTimestamp();
  d_debugger.timeThinkCycle(tLast);
  tLast = Debugger::getTimestamp();

  //
  // Capture the image
  //
  auto t = Debugger::getTimestamp();
  cv::Mat raw = d_camera.capture();

  d_debugger.timeImageCapture(t);

  d_streamer->streamImage(raw);

  //
  // Process the image
  //
  t = Debugger::getTimestamp();
  d_observations = processImage(raw);
  d_debugger.timeImageProcessing(t);

  //
  // Listen for any game control data
  //
  RoboCupGameControlData gameControlData;
  if (d_gameControlReceiver.receive(&gameControlData))
  {
    d_debugger.setGameControlData(gameControlData);
  }

  cout << "state: " << d_state << endl;

  switch (d_state)
  {
  case S_INIT:
    break;
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

  AgentModel::getInstance().state = d_state;

  //
  // Process input commands
  //
//processInputCommands();

  //
  // Get up, if we've fallen over
  //
  standUpIfFallen();

  //
  // Flush out new walking parameters
  //
  d_ambulator.step();

  //
  // Update LEDs on back, etc
  //
  d_debugger.update(d_CM730);

  //
  // Update websocket data
  //
  if (d_streamer != nullptr)
  {
    d_streamer->update();
  }

  t = Debugger::getTimestamp();
  readSubBoardData();
  d_debugger.timeSubBoardRead(t);
}
