#include "agent.ih"

Agent::Agent(std::string const& U2D_dev,
      std::string const& iniFile,
      std::string const& motionFile,
      bool const& showUI,
      bool const& useJoystick,
      bool const& autoGetUpFromFallen)
  : d_linuxCM730(U2D_dev.c_str()),
    d_CM730(&d_linuxCM730),
    d_ini(iniFile),
    d_motionFile(motionFile),
    d_camera(0),
    d_debugger(),
    d_ambulator(d_ini),
    d_minBallArea(8*8),
    d_joystick(nullptr),
    d_showUI(showUI),
    d_autoGetUpFromFallen(autoGetUpFromFallen),
    d_ballSeenCnt(0),
    d_goalSeenCnt(0)
{
  if (useJoystick)
  {
    d_joystick = new Joystick();
    d_joystickXAmpMax = d_ini.getd("Joystick", "XAmpMax", 15);
    d_joystickYAmpMax = d_ini.getd("Joystick", "YAmpMax", 15);
    d_joystickAAmpMax = d_ini.getd("Joystick", "AAmpMax", 15);
  }

  d_minBallArea = d_ini.geti("Vision", "MinBallArea", 8*8);

  // Example values
  //
  // gain:       1
  // brightness: 0.501961
  // contrast:   0.12549
  // saturation: 0.109804

  double gain       = d_ini.getd("Camera", "Gain",       d_camera.get(CV_CAP_PROP_GAIN));
  double brightness = d_ini.getd("Camera", "Brightness", d_camera.get(CV_CAP_PROP_BRIGHTNESS));
  double contrast   = d_ini.getd("Camera", "Contrast",   d_camera.get(CV_CAP_PROP_CONTRAST));
  double saturation = d_ini.getd("Camera", "Saturation", d_camera.get(CV_CAP_PROP_SATURATION));
//double hue        = d_ini.getd("Camera", "Hue",        d_camera.get(CV_CAP_PROP_HUE));
//double exposure   = d_ini.getd("Camera", "Exposure",   d_camera.get(CV_CAP_PROP_EXPOSURE));

  int width  = d_ini.geti("Camera", "Width", 320);
  int height = d_ini.geti("Camera", "Width", 240);

  cout << "[Agent::Agent] Using camera frame size: " << width << " x " << height << endl;
  cout << "[Agent::Agent] Using camera gain:       " << gain << endl;
  cout << "[Agent::Agent] Using camera brightness: " << brightness << endl;
  cout << "[Agent::Agent] Using camera contrast:   " << contrast << endl;
  cout << "[Agent::Agent] Using camera saturation: " << saturation << endl;
//cout << "[Agent::Agent] Using camera hue:        " << hue << endl;
//cout << "[Agent::Agent] Using camera exposure:   " << exposure << endl;

  d_camera.set(CV_CAP_PROP_FRAME_WIDTH,  width);
  d_camera.set(CV_CAP_PROP_FRAME_HEIGHT, height);
  d_camera.set(CV_CAP_PROP_GAIN,         gain);
  d_camera.set(CV_CAP_PROP_BRIGHTNESS,   brightness);
  d_camera.set(CV_CAP_PROP_CONTRAST,     contrast);
  d_camera.set(CV_CAP_PROP_SATURATION,   saturation);
//d_camera.set(CV_CAP_PROP_HUE,          hue);
//d_camera.set(CV_CAP_PROP_EXPOSURE,     exposure);
}

