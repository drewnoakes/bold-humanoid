#include "agent.hh"

using namespace bold;
using namespace std;

#include "../BehaviourControl/behaviourcontrol.hh"
#include "../Camera/camera.hh"
#include "../Debugger/debugger.hh"
#include "../DrawBridgeComms/drawbridgecomms.hh"
#include "../Drawing/drawing.hh"
#include "../GameStateReceiver/gamestatereceiver.hh"
#include "../Localiser/localiser.hh"
#include "../MotionTaskScheduler/motiontaskscheduler.hh"
#include "../OptionTree/optiontree.hh"
#include "../RoleDecider/roledecider.hh"
#include "../RemoteControl/remotecontrol.hh"
#include "../SequentialTimer/sequentialtimer.hh"
#include "../Spatialiser/spatialiser.hh"
#include "../State/state.hh"
#include "../StateObject/TimingState/timingstate.hh"
#include "../StateObserver/OpenTeamCommunicator/openteamcommunicator.hh"
#include "../VisualCortex/visualcortex.hh"

void Agent::onStep(ulong cycleNumber)
{
  ASSERT(ThreadUtil::isThinkLoopThread());

  log::trace("Agent::think") << "Starting think cycle " << cycleNumber << " --------------------------";

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
  d_visualCortex->integrateImage(image, t, cycleNumber);
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

  //
  // Refresh MotionTaskState, if one is needed
  //
  d_motionSchedule->update();
  t.timeEvent("Update Motion Schedule");

  //
  // Publish a snapshot of the behaviour control object
  //
  d_behaviourControl->updateStateObject();
  t.timeEvent("Publish Behaviour Control State");

  //
  // Invoke observers which requested to be called back from the think loop
  //
  t.enter("Observers");
  State::callbackObservers(ThreadId::ThinkLoop, t);
  t.exit();

  //
  // Send a message for drawbridge use within matches, containing status of the agent.
  //
  if (d_drawBridgeComms && cycleNumber % 30 == 0)
    d_drawBridgeComms->publish();

  // Flush out any drawing commands
  Draw::flushToStateObject();
  t.timeEvent("Flush Drawing Items");

  //
  // Set timing data for the think cycle
  //
  static FPS<30> fps;
  State::make<ThinkTimingState>(t.flush(), cycleNumber, fps.next());

  log::trace("Agent::think") << "Ending think cycle " << cycleNumber;
}
