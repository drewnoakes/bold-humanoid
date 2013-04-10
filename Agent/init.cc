#include "agent.ih"

bool Agent::init()
{
  cout << "[Agent::init] Start" << endl;

  initCamera();

  // Check if camera is opened successfully
  /*
  if (!d_camera.isOpened())
  {
    cout << "[Agent::init] Failed to open camera!" << endl;
    return false;
  }
  */

  // TODO only stream if argument specified?
  // TODO port from config, not constructor
  d_streamer = make_shared<DataStreamer>(8080);
  d_streamer->initialise(d_ini);
  d_streamer->setCamera(d_camera);

  d_streamer->registerControls("camera", d_camera->getControls());
  for (auto const& pair : d_visualCortex->getControlsByFamily())
    d_streamer->registerControls(pair.first, pair.second);

  Debugger::getInstance().update(d_CM730);

  // TODO source imageWidth, imageHeight, focalLength, rangeVertical, rangeHorizontal from config
  int imageWidth = 320;
  int imageHeight = 240;
  double focalLength = 0.025;
  double rangeVertical = 46/180.0 * M_PI;
  double rangeHorizontal = 58/180.0 * M_PI;;

  d_cameraModel = std::make_shared<CameraModel>(imageWidth, imageHeight, focalLength, rangeVertical, rangeHorizontal);

  d_agentModel = make_shared<AgentModel>();

  d_haveBody = initBody();

  d_state = State::S_INIT;

  cout << "[Agent::init] Done" << endl;

  return true;
}
