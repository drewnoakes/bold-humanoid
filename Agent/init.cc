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
  d_streamer = new DataStreamer(8080);
  d_streamer->initialise(d_ini);
  d_streamer->setCamera(d_camera);

  d_streamer->registerControls("camera", d_camera->getControls());
  for (auto const& pair : VisualCortex::getInstance().getControlsByFamily())
    d_streamer->registerControls(pair.first, pair.second);

  Debugger::getInstance().update(d_CM730);

  AgentModel::getInstance().initialise(d_ini);

  d_haveBody = initBody();

  d_state = S_INIT;

  cout << "[Agent::init] Done" << endl;

  return true;
}
