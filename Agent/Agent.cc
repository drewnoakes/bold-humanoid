#include "agent.ih"

Agent::Agent(std::string const& U2D_dev,
             minIni const& ini,
             std::string const& motionFile,
             bool const& useJoystick,
             bool const& autoGetUpFromFallen,
             bool const& recordFrames,
             unsigned int const& gameControlUdpPort
  )
  : d_ini(ini),
    d_motionFile(motionFile),
    d_isRecordingFrames(recordFrames),
    d_autoGetUpFromFallen(autoGetUpFromFallen),
    d_linuxCM730(U2D_dev.c_str()),
    d_CM730(&d_linuxCM730),
    d_ambulator(d_ini),
    d_gameControlReceiver(gameControlUdpPort),
    d_ballSeenCnt(0),
    d_goalSeenCnt(0)
{
  if (useJoystick)
  {
    d_joystick = make_shared<Joystick>(1);
    d_joystickXAmpMax = d_ini.getd("Joystick", "XAmpMax", 15);
    d_joystickYAmpMax = d_ini.getd("Joystick", "YAmpMax", 15);
    d_joystickAAmpMax = d_ini.getd("Joystick", "AAmpMax", 15);
  }

  d_circleBallX = d_ini.getd("Circle Ball", "WalkX", -1);
  d_circleBallY = d_ini.getd("Circle Ball", "WalkY", 50);
  d_circleBallTurn = d_ini.getd("Circle Ball", "WalkTurn", 15);

  d_camera = make_shared<Camera>("/dev/video0");
}

