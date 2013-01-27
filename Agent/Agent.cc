#include "agent.ih"

Agent::Agent(std::string const& U2D_dev,
      std::string const& iniFile,
      std::string const& motionFile,
      bool const& showUI,
      bool const& useJoystick)
  : d_linuxCM730(U2D_dev.c_str()),
    d_CM730(&d_linuxCM730),
    d_ini(iniFile),
    d_motionFile(motionFile),
    d_camera(0),
    d_debugger(),
    d_ambulator(),
    d_minBallArea(8*8),
    d_showUI(showUI),
    d_joystick(nullptr)
{
  if (useJoystick)
    d_joystick = new Joystick();

  int width = 320;
  int height = 240;
  cout << "Setting camera frame size to " << width << " x " << height << endl;
  d_camera.set(CV_CAP_PROP_FRAME_WIDTH, width);
  d_camera.set(CV_CAP_PROP_FRAME_HEIGHT, height);

  cout << "Current gain:       " << d_camera.get(CV_CAP_PROP_GAIN) << endl;
  cout << "Current brightness: " << d_camera.get(CV_CAP_PROP_BRIGHTNESS) << endl;
  cout << "Current contrast:   " << d_camera.get(CV_CAP_PROP_CONTRAST) << endl;
  cout << "Current saturation: " << d_camera.get(CV_CAP_PROP_SATURATION) << endl;
//cout << "Current hue:        " << d_camera.get(CV_CAP_PROP_HUE) << endl;
//cout << "Current exposure:   " << d_camera.get(CV_CAP_PROP_EXPOSURE) << endl;
}

