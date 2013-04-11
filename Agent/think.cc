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

  auto& debugger = Debugger::getInstance();

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

  if (d_streamer->shouldProvideImage())
    d_visualCortex->streamDebugImage(image, d_streamer);
  t = debugger.timeEvent(t, "Image Streaming");

  //
  // Listen for any game control data
  //
  RoboCupGameControlData gameControlData;
  if (d_gameControlReceiver.receive(&gameControlData))
  {
    AgentState::getInstance().setGameState(make_shared<GameState>(gameControlData));
    t = debugger.timeEvent(t, "Integrate Game Control");
  }

  OptionList options = {d_optionTree.getTop()};
  while (!options.empty())
  {
    OptionList subOptions = options.front()->runPolicy();
    options.pop_front();
    options.insert(options.end(), subOptions.begin(), subOptions.end());
  }

  /*
  switch (d_state)
  {
  case State::S_INIT:
    break;
    d_state = State::S_LOOK_FOR_BALL;

  case State::S_LOOK_FOR_BALL:
    lookForBall();
    break;

  case State::S_APPROACH_BALL:
    approachBall();
    break;

  case State::S_LOOK_FOR_GOAL:
    lookForGoal();
    break;

  case State::S_START_CIRCLE_BALL:
  case State::S_CIRCLE_BALL:
    circleBall();
    break;

  case State::S_START_PREKICK_LOOK:
  case State::S_PREKICK_LOOK:
    preKickLook();
    break;

  case State::S_KICK:
    kick();
    stand();

    break;

  case State::S_GET_UP:
    getUp();
    break;
  }
  */

//   AgentState::getInstance().state = d_state;
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
  d_ambulator->step();
  t = debugger.timeEvent(t, "Ambulator Step");

  //
  // Update LEDs on back, etc
  //
  debugger.update(d_CM730);
  t = debugger.timeEvent(t, "Update Debugger");

  //
  // Read all data from the sub board
  //
  readSubBoardData();
  t = debugger.timeEvent(t, "Read Sub Board");

  BodyState& body = *AgentState::getInstance().body();

  auto neck = body.getLimb("neck");
  auto neckHeadJoint = neck->joints[0];
  auto head = body.getLimb("head");
  auto cameraJoint = head->joints[0];
  auto camera = body.getLimb("camera");
  auto lFoot = body.getLimb("lFoot");
  auto lKnee = body.getLimb("lLowerLeg");
  auto rFoot = body.getLimb("rFoot");

  /*
  cout << "---------------" << endl;
  cout << "neckHeadJoint: " << neckHeadJoint->angle << endl << neckHeadJoint->transform.translation().transpose() << endl;
  cout << "head:" << endl << head->transform.translation().transpose() << endl;
  cout << "cameraJoint:" << endl << cameraJoint->transform.translation().transpose() << endl;
  cout << "camera:" << endl << camera->transform.matrix() << endl;
  cout << "foot: " << endl << lFoot->transform.matrix() << endl;
  auto cameraToLFoot = lFoot->transform.inverse() * camera->transform;
  cout << "cam2foot: " << endl << cameraToLFoot.translation().transpose() << endl;

  cout << "l foot transform:" << endl << lFoot->transform.matrix() << endl;
  cout << "l knee transform:" << endl << lKnee->transform.matrix() << endl;
  cout << "r foot transform:" << endl << rFoot->transform.matrix() << endl;
  */

  // TODO populate agent frame from camera frame

  double torsoHeight = lFoot->transform.inverse().translation().z();
  // Multiplying with this transform brings coordinates from camera space in torso space
  auto cameraTransform = camera->transform;

  auto const& ballObs = AgentState::getInstance().cameraFrame()->getBallObservation();
  if (ballObs.hasValue())
  {
    Spatialiser spatialiser(d_cameraModel);
    auto gp = spatialiser.findGroundPointForPixel(ballObs->cast<int>(), torsoHeight, camera->transform);
    //cout << "ground point: " << gp << endl;
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
