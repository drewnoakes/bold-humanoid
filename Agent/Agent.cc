#include "agent.ih"

#include "../Camera/camera.hh"
#include "../ImagePasser/imagepasser.hh"
#include "../ImagePassHandler/BlobDetectPass/blobdetectpass.hh"
#include "../ImagePassHandler/LineDotPass/linedotpass.hh"
#include "../LineFinder/linefinder.hh"

Agent::Agent(std::string const& U2D_dev,
      minIni const& ini,
      std::string const& motionFile,
      bool const& showUI,
      bool const& useJoystick,
      bool const& autoGetUpFromFallen,
      unsigned int const& gameControlUdpPort
  )
  : d_linuxCM730(U2D_dev.c_str()),
    d_CM730(&d_linuxCM730),
    d_ini(ini),
    d_motionFile(motionFile),
    d_ambulator(d_ini),
    d_autoGetUpFromFallen(autoGetUpFromFallen),
    d_gameControlReceiver(gameControlUdpPort),
    d_ballSeenCnt(0),
    d_goalSeenCnt(0),
    d_camera(nullptr),
    d_joystick(nullptr),
    d_visualCortex(nullptr),
    d_motionTimer(nullptr),
    d_streamer(nullptr)
{
  if (useJoystick)
  {
    d_joystick = new Joystick();
    d_joystickXAmpMax = d_ini.getd("Joystick", "XAmpMax", 15);
    d_joystickYAmpMax = d_ini.getd("Joystick", "YAmpMax", 15);
    d_joystickAAmpMax = d_ini.getd("Joystick", "AAmpMax", 15);
  }

  d_circleBallX = d_ini.getd("Circle Ball", "WalkX", -1);
  d_circleBallY = d_ini.getd("Circle Ball", "WalkY", 50);
  d_circleBallTurn = d_ini.getd("Circle Ball", "WalkTurn", 15);

  // Example values
  //
  // gain:       1
  // brightness: 0.501961
  // contrast:   0.12549
  // saturation: 0.109804

  d_camera = new Camera("/dev/video0");

//   double gain       = d_ini.getd("Camera", "Gain",       0);
//   double brightness = d_ini.getd("Camera", "Brightness", 0);
//   double contrast   = d_ini.getd("Camera", "Contrast",   0);
//   double saturation = d_ini.getd("Camera", "Saturation", 0);
// //double hue        = d_ini.getd("Camera", "Hue",        0);
// //double exposure   = d_ini.getd("Camera", "Exposure",   0);
//
//   int width  = d_ini.geti("Camera", "Width", 320);
//   int height = d_ini.geti("Camera", "Width", 240);
//
//   cout << "[Agent::Agent] Using camera frame size: " << width << " x " << height << endl;
//   cout << "[Agent::Agent] Using camera gain:       " << gain << endl;
//   cout << "[Agent::Agent] Using camera brightness: " << brightness << endl;
//   cout << "[Agent::Agent] Using camera contrast:   " << contrast << endl;
//   cout << "[Agent::Agent] Using camera saturation: " << saturation << endl;
// //cout << "[Agent::Agent] Using camera hue:        " << hue << endl;
// //cout << "[Agent::Agent] Using camera exposure:   " << exposure << endl;

  /*
  d_camera.set(CV_CAP_PROP_FRAME_WIDTH,  width);
  d_camera.set(CV_CAP_PROP_FRAME_HEIGHT, height);
  d_camera.set(CV_CAP_PROP_GAIN,         gain);
  d_camera.set(CV_CAP_PROP_BRIGHTNESS,   brightness);
  d_camera.set(CV_CAP_PROP_CONTRAST,     contrast);
  d_camera.set(CV_CAP_PROP_SATURATION,   saturation);
//d_camera.set(CV_CAP_PROP_HUE,          hue);
//d_camera.set(CV_CAP_PROP_EXPOSURE,     exposure);
*/
}

