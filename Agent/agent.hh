#ifndef BOLD_AGENT_HH
#define BOLD_AGENT_HH

#include <LinuxDARwIn.h>
#include <LinuxCM730.h>
#include <opencv.hpp>

#include <BlobDetector/blobdetector.hh>
#include "../Ambulator/ambulator.hh"
#include "../Debugger/debugger.hh"
#include "../joystick/joystick.hh"

namespace bold
{
  enum ObsType
  {
    O_BALL,
    O_GOAL_POST,
    O_LEFT_GOAL_POST,
    O_RIGHT_GOAL_POST
  };

  struct Observation
  {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    ObsType type;
    Eigen::Vector2f pos;
  };

  class Agent
  {
  public:
    Agent(std::string const& U2D_dev,
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
    }

    int run();

  private:
    Robot::LinuxCM730 d_linuxCM730;
    Robot::CM730 d_CM730;

    Robot::LinuxMotionTimer* d_motionTimer;

    minIni d_ini;

    std::string d_motionFile;

    cv::VideoCapture d_camera;

    Debugger d_debugger;

    unsigned char* d_LUT;

    BlobDetector d_blobDetector;

    Ambulator d_ambulator;

    int d_minBallArea;

    Joystick* d_joystick;

    bool d_showUI;

    bool init();

    void think();

    std::vector<Observation> processImage(cv::Mat& image);
  };
}

#endif
