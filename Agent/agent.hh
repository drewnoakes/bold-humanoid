#ifndef AGENT_HH
#define AGENT_HH

#include <LinuxDARwIn.h>
#include <LinuxCM730.h>

#include <opencv.hpp>

#include <BlobDetector/blobdetector.hh>

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
	  std::string const& motionFile)
      : d_linuxCM730(U2D_dev.c_str()),
	      d_CM730(&d_linuxCM730),
        d_ini(iniFile),
        d_motionFile(motionFile),
        d_camera(0),
        d_stepCurrent(0),
        d_stepTarget(0),
        d_stepChangeAmount(0.3),
        d_stepMax(10.0),
        d_turnCurrent(0),
        d_turnTarget(0),
        d_turnChangeAmount(1.0),
        d_turnMax(15.0)
    {}

    void run();

  private:
    Robot::LinuxCM730 d_linuxCM730;
    Robot::CM730 d_CM730;

    Robot::LinuxMotionTimer* d_motionTimer;

    minIni d_ini;

    std::string d_motionFile;

    cv::VideoCapture d_camera;

    unsigned char* d_LUT;

    BlobDetector d_blobDetector;

    double d_stepCurrent;
    double d_stepTarget;
    double d_stepMax;
    double d_stepChangeAmount;

    double d_turnCurrent;
    double d_turnTarget;
    double d_turnMax;
    double d_turnChangeAmount;

    bool init();

    void think();

    std::vector<Observation> processImage(cv::Mat& image);
  };
}

#endif
