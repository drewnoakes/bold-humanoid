#include "agent.ih"
#include "../AgentModel/agentmodel.hh"
#include "../GameController/RoboCupGameControlData.h"

void Agent::think()
{
  //
  // Initialise a time value that we'll repeatedly use and update to time
  // the progress of each step of the think cycle
  //
  auto t = Debugger::getTimestamp();

  auto& debugger = Debugger::getInstance();

  //
  // Capture the image
  //
  cv::Mat raw = d_camera.capture();
  t = debugger.timeEvent(t, "Image Capture");

  d_pfChain.applyFilters(raw);
  t = debugger.timeEvent(t, "Pixel Filter Chain");

  d_streamer->streamImage(raw);
  t = debugger.timeEvent(t, "Stream Image");

  //
  // Process the image
  //
  WorldModel::getInstance().integrateImage(raw);
  t = debugger.timeEvent(t, "Process Image");

  //
  // Listen for any game control data
  //
  RoboCupGameControlData gameControlData;
  if (d_gameControlReceiver.receive(&gameControlData))
  {
    GameState::getInstance().update(gameControlData);
    t = debugger.timeEvent(t, "Integrate Game Control");
  }

//  cout << "state: " << d_state << endl;

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

  t = debugger.timeEvent(t, "Process State");

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
  d_ambulator.step();
  t = debugger.timeEvent(t, "Ambulator Step");

  //
  // Update LEDs on back, etc
  //
  debugger.update(d_CM730);
  t = debugger.timeEvent(t, "Update Debugger");

  //
  // Update websocket data
  //
  if (d_streamer != nullptr)
  {
    d_streamer->update();
    t = debugger.timeEvent(t, "Update DataStreamer");
  }

  readSubBoardData();
  t = debugger.timeEvent(t, "Read Sub Board");
}
