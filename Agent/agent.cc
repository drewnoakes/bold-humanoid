#include "agent.hh"

#include "../BehaviourControl/behaviourcontrol.hh"
#include "../Camera/camera.hh"
#include "../CameraModel/cameramodel.hh"
#include "../CM730CommsModule/MX28HealthChecker/mx28healthchecker.hh"
#include "../DataStreamer/datastreamer.hh"
#include "../DebugControl/debugcontrol.hh"
#include "../Debugger/debugger.hh"
#include "../DrawBridgeComms/drawbridgecomms.hh"
#include "../Drawing/drawing.hh"
#include "../GameStateReceiver/gamestatereceiver.hh"
#include "../ImageLabeller/imagelabeller.hh"
#include "../Kick/kick.hh"
#include "../Localiser/localiser.hh"
#include "../MotionLoop/motionloop.hh"
#include "../MotionModule/HeadModule/headmodule.hh"
#include "../MotionModule/MotionScriptModule/motionscriptmodule.hh"
#include "../MotionModule/WalkModule/walkmodule.hh"
#include "../MotionTaskScheduler/motiontaskscheduler.hh"
#include "../RemoteControl/remotecontrol.hh"
#include "../RoleDecider/roledecider.hh"
#include "../State/state.hh"
#include "../StateObserver/ButtonObserver/buttonobserver.hh"
#include "../StateObserver/FallDetector/AccelerometerFallDetector/accelerometerfalldetector.hh"
#include "../StateObserver/FallDetector/OrientationFallDetector/orientationfalldetector.hh"
#include "../StateObserver/GyroCalibrator/gyrocalibrator.hh"
#include "../StateObserver/HandlerHelper/handlerhelper.hh"
#include "../StateObserver/HealthAndSafety/healthandsafety.hh"
#include "../StateObserver/JamDetector/jamdetector.hh"
#include "../StateObserver/Odometer/odometer.hh"
#include "../StateObserver/OrientationTracker/orientationtracker.hh"
#include "../StateObserver/OpenTeamCommunicator/openteamcommunicator.hh"
#include "../StateObserver/StationaryMapper/stationarymapper.hh"
#include "../StateObserver/SuicidePill/suicidepill.hh"
#include "../StateObserver/Vocaliser/vocaliser.hh"
#include "../Spatialiser/spatialiser.hh"
#include "../VisualCortex/visualcortex.hh"
#include "../Voice/voice.hh"

using namespace std;
using namespace bold;

Agent::Agent()
  : Loop("Think Loop", SCHED_OTHER, 0),
    d_isShutdownRequested(false),
    d_teamNumber((uchar)Config::getStaticValue<int>("team-number")),
    d_uniformNumber((uchar)Config::getStaticValue<int>("uniform-number")),
    d_teamColour(Config::getStaticValue<TeamColour>("team-colour")),
    d_startTime(Clock::getTimestamp())
{
  ThreadUtil::setThreadId(ThreadId::ThinkLoop);

  log::info("Agent::Agent") << "Name:    " << Config::getStaticValue<string>("player-name");
  log::info("Agent::Agent") << "Uniform: " << (int)d_uniformNumber;
  log::info("Agent::Agent") << "Team:    " << (int)d_teamNumber;
  log::info("Agent::Agent") << "Colour:  " << (d_teamColour == TeamColour::Cyan ? "Cyan" : "Magenta");

  State::initialise();

  FieldMap::initialise();

  Kick::loadAll();

  d_voice = make_shared<Voice>();
  d_behaviourControl = make_shared<BehaviourControl>(*this);

  registerStateTypes();

  log::logGameState = true;

  d_buttonObserver = make_shared<ButtonObserver>();

  auto debugControl = make_shared<DebugControl>();
  d_debugger = make_shared<Debugger>(this, d_behaviourControl, debugControl, d_voice, d_buttonObserver);

  // Prepare the motion schedule, that coordinates which motions are carried out
  d_motionSchedule = make_shared<MotionTaskScheduler>();

  // Create motion modules
  d_headModule = make_shared<HeadModule>(d_motionSchedule);
  d_walkModule = make_shared<WalkModule>(d_motionSchedule);
  d_motionScriptModule = make_shared<MotionScriptModule>(d_motionSchedule);

  MotionScriptModule::createActions("./motionscripts", d_motionScriptModule);

  // Create StateObservers
  d_vocaliser = make_shared<Vocaliser>(d_voice);
  d_gyroCalibrator = make_shared<GyroCalibrator>();
  d_healthAndSafety = make_shared<HealthAndSafety>(d_voice);
  d_jamTracker = make_shared<JamDetector>(d_voice);
  d_suicidePill = make_shared<SuicidePill>(this, d_debugger);
  d_odometer = make_shared<Odometer>(d_walkModule);
  d_orientationTracker = make_shared<OrientationTracker>();
  d_teamCommunicator = make_shared<OpenTeamCommunicator>(d_behaviourControl);
  auto stationaryMapper = make_shared<StationaryMapper>(d_voice, d_behaviourControl);
  auto handlerHelper = make_shared<HandlerHelper>(d_voice, d_behaviourControl);

  // Register StateObservers
  State::registerObserver(d_buttonObserver);
  State::registerObserver(d_vocaliser);
  State::registerObserver(d_gyroCalibrator);
  State::registerObserver(d_healthAndSafety);
  State::registerObserver(d_jamTracker);
  State::registerObserver(d_suicidePill);
  State::registerObserver(d_odometer);
  State::registerObserver(d_teamCommunicator);
  State::registerObserver(d_orientationTracker);
  State::registerObserver(stationaryMapper);
  State::registerObserver(handlerHelper);

  // Special handling for fall detection based upon config
  switch (Config::getStaticValue<FallDetectorTechnique>("fall-detector.technique"))
  {
    case FallDetectorTechnique::Accelerometer:
    {
      auto fallDetector = make_shared<AccelerometerFallDetector>(d_voice);
      d_fallDetector = fallDetector;
      State::registerObserver(fallDetector);
      break;
    }
    case FallDetectorTechnique::Orientation:
    {
      auto fallDetector = make_shared<OrientationFallDetector>(d_voice);
      d_fallDetector = fallDetector;
      State::registerObserver(fallDetector);
      break;
    }
  }

  d_cameraModel = allocate_aligned_shared<CameraModel>();

  d_spatialiser = make_shared<Spatialiser>(d_cameraModel);

  d_localiser = allocate_aligned_shared<Localiser>();

  d_roleDecider = make_shared<RoleDecider>(d_behaviourControl, d_voice);

  // Create camera
  d_camera = make_shared<Camera>(Config::getStaticValue<string>("hardware.video-path"));
  d_camera->open();

  // Configure camera
  unsigned width = d_cameraModel->imageWidth();
  unsigned height = d_cameraModel->imageHeight();
  if (!d_camera->getPixelFormat().requestSize(width, height))
    log::error() << "[Agent::initCamera] Requesting camera size " << width << "x" << height << " failed";

  auto pixelFormat = d_camera->getPixelFormat();
  log::info("Agent::initCamera") << "Camera resolution: " << pixelFormat.width << "x" << pixelFormat.height;
  log::info("Agent::initCamera") << "Pixel format: " << pixelFormat.pixelFormatString();
  log::info("Agent::initCamera") << "Bytes per line: " << pixelFormat.bytesPerLine;
  log::info("Agent::initCamera") << "imageByteSize: " << pixelFormat.imageByteSize;


  // Start capturing camera images
  d_camera->startCapture();

  d_streamer = make_shared<DataStreamer>(d_camera);

  d_visualCortex = make_shared<VisualCortex>(d_camera, d_cameraModel, d_streamer, d_spatialiser, d_headModule);

  d_gameStateReceiver = make_shared<GameStateReceiver>(d_debugger, d_voice);

  d_remoteControl = make_shared<RemoteControl>(this);

  if (Config::getStaticValue<bool>("drawbridge.enabled"))
    d_drawBridgeComms = make_shared<DrawBridgeComms>(this, d_behaviourControl, d_debugger);

  d_debugger->update();

  Draw::initialise();

  d_motionLoop = make_shared<MotionLoop>(debugControl);
  d_motionLoop->addMotionModule(d_motionScriptModule);
  d_motionLoop->addMotionModule(d_walkModule);
  d_motionLoop->addMotionModule(d_headModule);

  auto mx28HealthChecker = make_shared<MX28HealthChecker>(d_voice);
  d_motionLoop->addCommsModule(mx28HealthChecker);

  d_motionLoop->onReadFailure.connect([this](uint count) -> void {
    // If we were unable to read once during the last second, then we've lost
    // the CM730. This is probably because someone pressed the hardware reset
    // button on the back. In this case, stop the agent. The upstart service
    // should restart it immediately during competitions.
    // TODO consider reconnecting to the CM730, as we already have a means of exiting the robot by holding down two buttons
    // TODO place read failure timeout duration in config
    if (count > 1000 / 8) // 1 second
    {
      log::error("Agent") << "MotionLoop has been unable to read " << count << " times, so shutting down";
      exit(EXIT_FAILURE);
    }
  });
}

ulong Agent::getThinkCycleNumber() const
{
  return getCycleNumber();
}

void Agent::setOptionTree(shared_ptr<OptionTree> tree)
{
  log::verbose("Agent::setOptionTree") << "Setting OptionTree";
  d_optionTree = tree;
  d_streamer->setOptionTree(tree);
}

void Agent::requestStop()
{
  if (!isRunning())
    throw runtime_error("Not running");

  if (d_isShutdownRequested)
  {
    log::warning("Agent::requestStop") << "Forcing exit";
    exit(EXIT_FAILURE);
  }

  log::info("Agent::requestStop");

  // The option tree picks up this flag and performs a shutdown sequence
  d_isShutdownRequested = true;

  d_voice->say("Shutting down");
}

void Agent::onStopped()
{
  d_streamer->stop();

  if (d_motionLoop)
    d_motionLoop->stop();

  if (d_voice)
    d_voice->stop();

  log::info("Agent::stop") << "Stopped";
}
