#include "agent.ih"

Agent::Agent()
  : Configurable("agent"),
    d_isRunning(false)
{
  cout << "[Agent::Agent] Start" << endl;

  cout << "U2D dev name: " << getParam("u2dDevName") << endl;

  /*
  registerStateTypes();

  d_linuxCM730 = make_shared<LinuxCM730>(U2D_dev.c_str());
  d_CM730 = make_shared<CM730>(d_linuxCM730.get());
  d_CM730->MakeBulkReadPacket();

  d_ambulator = make_shared<Ambulator>(d_ini),

  d_cameraModel = make_shared<CameraModel>(d_ini);

  d_spatialiser = make_shared<Spatialiser>(d_cameraModel);

  d_fieldMap = make_shared<FieldMap>(d_ini);

  d_debugger = make_shared<Debugger>();

  d_localiser = make_shared<Localiser>(d_fieldMap, d_ini);

  d_visualCortex = make_shared<VisualCortex>(d_cameraModel, d_fieldMap, d_spatialiser, d_debugger, d_ini);

  d_gameStateReceiver = make_shared<GameStateReceiver>(d_ini, d_debugger);

  if (useJoystick)
  {
    d_joystick = make_shared<Joystick>(1);
    d_joystickXAmpMax = d_ini.getd("Joystick", "XAmpMax", 15);
    d_joystickYAmpMax = d_ini.getd("Joystick", "YAmpMax", 15);
    d_joystickAAmpMax = d_ini.getd("Joystick", "AAmpMax", 15);
  }

  initCamera(d_ini);

  // TODO only stream if argument specified?
  d_streamer = make_shared<DataStreamer>(d_ini, d_camera, d_debugger);

  // TODO a better abstraction over control providers
  d_streamer->registerControls("camera", d_camera->getControls());
  d_streamer->registerControls("localiser", d_localiser->getControls());
  d_streamer->registerControls("ambulator", d_ambulator->getControls());
  for (auto const& pair : d_visualCortex->getControlsByFamily())
    d_streamer->registerControls(pair.first, pair.second);

  d_debugger->update(d_CM730);

  d_haveBody = initMotionManager(d_ini);
  */

  cout << "[Agent::Agent] Done" << endl;
}
